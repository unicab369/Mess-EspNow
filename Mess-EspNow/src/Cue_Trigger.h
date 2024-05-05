#define DEBUG_ESPNOW 1

enum Cue_Trigger: uint8_t {
   TRIGGER_STARTUP = 0xB0,
   TRIGGER_SINGLECLICK = 0xB1,
   TRIGGER_DOUBLECLICK = 0xB2,
   TRIGGER_PIR = 0xB3,
   TRIGGER_IR = 0xB4,
   TRIGGER_STATE = 0xB5,
   TRIGGER_NONE = 0xF1,
};

void AppPrintHex(void *data, int len, char separator = ' ') {
   uint8_t* arr = (uint8_t*)data;

   for (int i=0; i<len; i++) {
      Serial.printf("0x%02X", arr[i]);
      if (i<len-1) Serial.print(separator);
   }

   Serial.println();
}
