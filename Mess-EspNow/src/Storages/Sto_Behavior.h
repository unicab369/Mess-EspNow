
template <class T, uint8_t count>
class Sto_Array {
   protected:
      EEPROM_Value<T> objects[count];
      bool isLoaded = false; 
      uint16_t arrAddress;

      // Sto_Array(const char* id): Loggable(id) {}
      
   public:
      T* getValueAt(uint8_t index) {
         return objects[index].getValue();
      }

      //! loadData
      void loadData(uint16_t arrAddr) {
         // xLogSectionf("%s count = %u", __func__, count);
         arrAddress = arrAddr;
      
         for (int i=0; i<count; i++) {
            //! offset by 2 = 1 for the checkByte + 1 for array element offset
            uint16_t addr = arrAddress + i*(sizeof(T)+2);
            objects[i].loadData(addr);
         }

         // xLogSection("Print All Data\n");
         // AppPrintHex(objects, 124);
         isLoaded = true;
      }

      //! updateData
      void updateData(uint8_t index, T* newItem) {
         if (index>=count || !isLoaded) return;
         objects[index].updateData(newItem);
         loadData(arrAddress);
      }

      //! deleteData
      void deleteData() {
         if (!isLoaded) return;
         for (int i=0; i<count; i++) {
            objects[i].deleteData();
         }
         loadData(arrAddress);   
      }

      void forEach(std::function<void(T*, uint8_t index)> cb) {
         if (!isLoaded) return;
         
         for (int i=0; i<count; i++) {
            cb(getValueAt(i), i);
         }
      }

      T* firstMatch(std::function<bool(T*, uint8_t index)> cb) {
         if (!isLoaded) return NULL;

         for (int i=0; i<count; i++) {
            T* target = getValueAt(i);
            bool match = cb(target, i);
            if (match) return target;
         }

         return NULL;
      }

};

//! This object get stored in EEPROM
//! please keep size minimal, dont inherit Loggable
struct Data_Peer {
   uint8_t mac[6];
   uint8_t peerId = 255;
   uint64_t builtTime = 0;

   Data_Peer() {}
   
   Data_Peer(const uint8_t macVal[6], uint64_t id = 0) {
      memcpy(mac, macVal, sizeof(mac));
      peerId = id;
   }

   //! isValid if peerId is not 255 && mac is not 00:00:00:00:00:00
   bool isValid() {
      uint8_t macZero[6] = {};
      bool isMacZero = memcmp(mac, macZero, sizeof(mac)) == 0;
      return peerId != 255 && !isMacZero;
   }

   bool hasSameMac(uint8_t* targetMac) {
      bool compare = memcmp(mac, targetMac, sizeof(mac)) == 0;
      return compare;
   }

   void clear() {
      peerId = 255;
      memset(mac, 0, sizeof(mac));
   }

   void printData() {
      Serial.printf("Mac = %02X:%02X:%02X:%02X:%02X:%02X", 
                           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      Serial.printf(" peerId = %u", peerId);
   }
};

#define MAX_PEER_COUNT 5

class Sto_Peer: public Sto_Array<Data_Peer, MAX_PEER_COUNT> {
   public:
      // Sto_Peer(): Sto_Array("Sto_Peer") {}
      
      void printAll() {
         // xLogSection(__func__);
         forEach([&](Data_Peer* item, uint8_t index) {
            item->printData();
            Serial.println();
         });
      }

      void insertPeer(uint8_t* peerMac, uint8_t index) {
         Data_Peer newPeer(peerMac, index);
         newPeer.builtTime = 0x1122334455667788;
         updateData(index, &newPeer);
         printAll();
      }

      uint8_t addPeer(uint8_t* peerMac) {
         // xLogf("%s %02X:%02X:%02X:%02X:%02X:%02X", __func__,
                     // peerMac[0], peerMac[1], peerMac[2], 
                     // peerMac[3], peerMac[4], peerMac[5]);
         uint8_t lastAvailIndex = 255;

         Data_Peer* match = firstMatch([&](Data_Peer* item, uint8_t index) {
            bool foundMatch = item->hasSameMac(peerMac);

            if (foundMatch) {
               return true;
            } else if (!item->isValid()) {
               lastAvailIndex = index;
            }

            return false;
         });

         if (match != nullptr) {
            // xLogf("foundIndex = %u", match->peerId);
            return match->peerId;

         } else if (lastAvailIndex != 255) {
            // xLogf("**ADD NEW PEER at Index = %u", lastAvailIndex);
            Data_Peer newPeer(peerMac);
            newPeer.peerId = lastAvailIndex;
            newPeer.builtTime = 0x1122334455667788;
            updateData(lastAvailIndex, &newPeer);
         } else {
            // xLogf("**NO AVAILABLE SPOT");
         }

         printAll();
         return lastAvailIndex;
      }

      // uint8_t findPeer(uint8_t* targetMac) {
      //    forEach([&](Data_Peer* item, uint8_t index) {
      //       if (item->hasSameMac(targetMac)) {
      //          return item->peerId;
      //       }
      //    });

      //    return 255;
      // }

      bool handleCommand(char* input) {
         char refStr[64] = "";

         if (strcmp(input, "peers") == 0) {
            printAll();
            return true;
         }
         else if (strcmp(input, "peerDelAll") == 0) {
            deleteData();
            printAll();
            return true;
         }
         else if (extractValue("peerAdd", input, refStr)) {
            uint8_t mac[6] = { 0 };
            sscanf(refStr, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
            addPeer(mac);
            printAll();
            return true;
         }
         else if (extractValue("peerDel", input, refStr)) {
            int intValue = String(refStr).toInt();
            objects[intValue].value.clear();
            printAll();
            return true;
         }
         return false;
      }
};

#define MAX_BEHAVIOR_ITEMS 6

class Sto_Behavior: public Sto_Array<Data_Behavior, MAX_BEHAVIOR_ITEMS> {
   public:
      // Sto_Behavior(): Sto_Array("Sto_Behav") {}
      
      void printAll() {
         // xLogSection(__func__);
         forEach([&](Data_Behavior* item, uint8_t index) {
            item->printData();
         });
      }

      void handleCue(uint8_t peerId, Cue_Trigger cue) {
         forEach([&](Data_Behavior* item, uint8_t index) {
            // xLogf("At Index = %u", index);
            if (item->check(peerId, cue) == false) return;      
            ControlOutput out1(0, 0);
            ControlWS2812 out2(0, 0);

            if (out1.extract(item)) {
               // xLogf("IM HERE1 pin = %u, value = %u", out1.pin, out1.value);
            } else if (out2.extract(item)) {
               // xLogf("IM HERE2 pin = %u, value = %u", out2.pin, out2.value);
            }
         });
      }

      template <Cue_Trigger trigger, class T>
      void storeAction(uint8_t behavIndex, T* action, uint8_t peerId) {
			// uint8_t id = stoPeer.addPeer(peerMac);
			Data_Behavior behav_In;
			behav_In.load(peerId, trigger, action);
			updateData(behavIndex, &behav_In);     //! store behavior
         printAll();
		}

      bool handleCommand(char* input) {
         char refStr[64] = "";

         if (strcmp(input, "behav") == 0) {
            printAll();
            return true;
         }
         else if (strcmp(input, "behavDelAll") == 0) {
            deleteData();
            printAll();
            return true;
         }
         else if (extractValue("behavAdd", input, refStr)) {
            uint8_t behavIndex = 255, peerId = 255;
            char cmdStr[20] = "";
            if (refStr != nullptr && strlen(refStr)>0) {
               Serial.printf("\nIM HERE ************************* = %u", strlen(refStr));
               sscanf(refStr, "%u %u %s", &behavIndex, &peerId, &cmdStr);
               Serial.printf("\nbehavIndex = %u; peerId = %u; cmdStr = %s", behavIndex, peerId, cmdStr);
               return true;
            }
            
            return false;
         }
         else if (extractValue("behavDel", input, refStr)) {
            int intValue = String(refStr).toInt();
            objects[intValue].value.clear();
            printAll();
            return true;
         }
         return false;
      }
};
