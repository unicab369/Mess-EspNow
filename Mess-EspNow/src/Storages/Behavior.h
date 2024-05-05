enum Cmd_Behavior: uint8_t {
   ACTION_OUTPUT = 0xA0,
   ACTION_WS2812 = 0xA1,
   ACTION_REPORT = 0xA2,
   ACTION_SEND = 0xA3,
   ACTION_NONE = 0xAF
};

struct Data_Behavior {
   Cue_Trigger cue;
   uint8_t peerId;
   uint8_t data[32]; 
   
   void toCharArr(char* charArr) const {
      memcpy(charArr, this, sizeof(Data_Behavior));
   }

   bool check(uint8_t peerIdIdVal, Cue_Trigger cueVal) {
      return peerId == peerIdIdVal && cue == cueVal;
   }
   
   template <typename T>
   void load(uint8_t peerIdVal, Cue_Trigger cueVal, T* control) {
      memcpy(data, control, sizeof(T));
      peerId = peerIdVal;
      cue = cueVal;
   }

   template <typename T>
   void load(uint8_t peerIdVal, const char* cueStr, T* control) {
      Cue_Trigger trigger = TRIGGER_NONE;

      if (strcmp(cueStr, "1CLICK") == 0) {
         trigger = TRIGGER_SINGLECLICK;
      }
      else if (strcmp(cueStr, "2CLICK") == 0) {
         trigger = TRIGGER_DOUBLECLICK;
      }
      else if (strcmp(cueStr, "CUE_PIR") == 0) {
         trigger = TRIGGER_PIR;
      }
      else if (strcmp(cueStr, "CUE_IR") == 0) {
         trigger = TRIGGER_IR;
      }
      else if (strcmp(cueStr, "CUE_STATE") == 0) {
         trigger = TRIGGER_STATE;
      }
      else if (strcmp(cueStr, "CUE_THRESHOLD") == 0) {
         trigger = TRIGGER_NONE;
      }

      load(peerIdVal, trigger, control);
   }

   template <typename T>
   void produce(T *control) {
      memcpy(control, data, sizeof(T));
   }

   void clear() {
      peerId = 255;
      cue = TRIGGER_NONE;
      memset(data, 0, sizeof(data));
   }

   void printData() {
      Serial.printf("cue = %u, peerId = %u", cue, peerId);
      // AppPrintHex(data, sizeof(data));
   }
};

class ControlOutput {
   Cmd_Behavior actionCmd = ACTION_OUTPUT;

   public:
      uint8_t pin, value;

      ControlOutput(uint8_t pinVal, uint8_t valueVal) {
         pin = pinVal;
         value = valueVal;
      }

      bool extract(Data_Behavior* behav) {
         behav->produce(this);
         return actionCmd == ACTION_OUTPUT;
      }
};

class ControlWS2812 {
   Cmd_Behavior actionCmd = ACTION_WS2812;

   public:
      uint8_t pin, value;

      ControlWS2812(uint8_t pinVal, uint8_t valueVal) {
         pin = pinVal;
         value = valueVal;
      }


      bool extract(Data_Behavior* behav) {
         behav->produce(this);
         return actionCmd == ACTION_WS2812;
      }
};

class ControlReport {
   public:
      float value1, value2, value3;
};

class ControlSend {
   public:
      char myStr[16] = "testStr";
      Cmd_Behavior actionCmd = ACTION_SEND;
};

