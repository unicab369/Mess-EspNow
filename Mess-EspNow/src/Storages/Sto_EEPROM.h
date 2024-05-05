#include <EEPROM.h>

// [0xDD, 0xDD, resetCnt_1, resetCnt_0 ....] 0-64
// [password] 65-140
// [name] 141-175
// [mqttServer] 176- 200
// [mqttTopic] 201-270

class Sto_EEPROM {
   protected:
      const char name[20] = "EEPROM";
      // Loggable logger = Loggable(name);

      void writeValue(uint16_t address, uint64_t value) {
         for (int i = 0; i < 8; i++) {
               EEPROM.write(address + i, (uint8_t)(value >> (i * 8)));
         }
         EEPROM.commit(); // Commit the changes to EEPROM            
      }

      void readValue(uint16_t address, uint64_t* value) {
         *value = 0;
         for (int i = 0; i < 8; i++) {
               *value |= ((uint64_t)EEPROM.read(address + i) << (i * 8));
         }            
      }

      void deleteBytes(uint16_t address, uint8_t value, size_t len) {
         // logger.xLogf("%s @addr = %u", __func__, address);
         for (int i=0; i<len; i++) {
               EEPROM.write(address+i, value);
         }
         EEPROM.commit();
      }

      void writeBytes(uint16_t address, const void *value, size_t len) {
         // logger.xLogf("%s @addr = %u", __func__, address);
         byte* val = (byte*) value;
      
         for (int i=0; i<len; i++) {
               EEPROM.write(address+i, val[i]);
         }
         EEPROM.commit();

         //! for testing
         byte data[len];
         readBytes(address, data, len);
      }

      void readBytes(uint16_t address, void *value, size_t len) {
         // logger.xLogLinef("%s @addr = %u len = %zu", __func__, address, len);
         byte* val = (byte*) value;

         for (int i=0; i<len; i++) {
               val[i] = EEPROM.read(address+i);
         }

         // AppPrintHex(val, len);
      }

      void storeData(uint16_t address, const char *buf, size_t len) {
         for (int i=0; i<len; i++) {
               EEPROM.write(address+i, buf[i]);
         }

         EEPROM.commit();        
      }

      void writeByte(uint16_t address, uint8_t value) {
         EEPROM.write(address, value);
         EEPROM.commit(); 
      }

      uint8_t readByte(uint16_t address) {
         return EEPROM.read(address);
      }
};


bool extractValue(const char* key, char* input, char* output) {
   //! WARNING: strtok detroys the original string, perform operation on copied string
   char inputStr[240] = "";
   memcpy(inputStr, input, sizeof(inputStr));
   char *ref = strtok(inputStr, " ");     // get key

   //! check key
   if (ref != nullptr && strlen(ref)>0 && strcmp(ref, key) == 0) {
      ref = strtok(NULL, "");            // get value
      strcpy(output, ref);
      return true;
   } 

   return false;
}


template <class T>
class EEPROM_Value: public Sto_EEPROM {
   protected:
      void writeCode() {
         writeByte(startAddr, 0xDD);
      }   

      bool checkCode() {
         return readByte(startAddr) == 0xDD;
      }

      void clearCode() {
         writeByte(startAddr, 0x00);
      }

      //! startAddr contains checkByte, content follows startAddr
      uint16_t contentAddr() { return startAddr + 1; }

   public:
      uint16_t startAddr = 0;
      T value;
      T* getValue() { return &value; }

      //! loadData
      bool loadData(uint16_t addr) {
         startAddr = addr;
         if (!checkCode()) return false;
         readBytes(contentAddr(), &value, sizeof(T)+1);
         return true;
      }

      //! storeData
      void storeData(bool log = true) {
         writeCode();
         writeBytes(contentAddr(), &value, sizeof(T)+1);
         if (log) {
            (&value)->printData();
         }
      }

      //! deleteData
      void deleteData() {
         writeCode();
         deleteBytes(contentAddr(), 0, sizeof(T)+1);
      }

      //! updateData
      void updateData(T* newValue) {
         writeCode();
         writeBytes(contentAddr(), newValue, sizeof(T)+1);
      }

      bool storeValue(const char* key, char* input, char* output) {
         bool check = extractValue(key, input, output);
         if (check) storeData();
         return check;
      }

      bool storeValue(const char* key, char* input, bool* output) {
         char boolStr[2];

         bool check = extractValue(key, input, boolStr);
         if (check) {
            *output = strcmp("1", boolStr) == 0;
            storeData();
         }

         return check;
      }

      bool storeValue(const char* key, char* input, uint8_t* output, 
               std::function<bool(uint8_t)> validate = [](uint8_t) { return true; }) {
         char valStr[16];

         bool check = extractValue(key, input, valStr);
         if (check) {
            uint8_t value = (uint8_t)atoi(valStr);
            if (validate(value)) {
               *output = value;
               storeData();
            }
         }

         return check;
      }
};