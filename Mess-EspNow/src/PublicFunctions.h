// #define DEBUG_ESPNOW 1
#define MS_INTERVAL 50

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

class ControlTimer {
   unsigned long timeRef = millis();

   //! update tick every second
   bool updateTick() {
      if (t_millis < 1000) return false;
      // update second
      t_millis = 0;
      secChanged = true;
      t_secs++;

      if (t_secs >= 60) {
            // update Minutes
            t_secs = 0;
            minChanged = true;
            t_mins++;
            
            if (t_mins >= 60) {
               // update Hours
               t_mins = 0;
               hrChanged = true;
               t_hrs++;
            }
      }

      return true;
   }

   //! reset tick every second
   void resetTick() {
      secChanged = false;
      minChanged = false;
      hrChanged = false;
      cycleCount = 0;
      maxCycleTime = 0;
   }

   public:
      //! NOTE: DO NOT overwrite outside of class
      int t_secs = 0, t_mins = 0; 
      uint32_t t_millis = 0, t_hrs = 0;

      bool secChanged = false, minChanged = false, hrChanged = false;

      uint32_t cycleCount = 0;          //! Cycle count per second
      unsigned long maxCycleTime = 0;

      bool isHundredMsInterval(int value) {
         return t_millis%value == 0;
      }
      
      bool isSecondInterval(int value) { 
         return secChanged && t_secs%value == 0;
      }

      bool isMinuteInterval(int value) { 
         return minChanged && t_mins%value == 0; 
      }

      void run(std::function<void()>callback) {
         cycleCount++;
         unsigned long newTime = millis();
         unsigned long timeDif = newTime - timeRef;

         if (timeDif > MS_INTERVAL) {
            t_millis+=MS_INTERVAL;
            timeRef = millis();
         }

         bool check = updateTick();
         callback();
         if (check) resetTick();

         unsigned long cycleTime = millis() - newTime;
         if (cycleTime > maxCycleTime) maxCycleTime = cycleTime;
      }
};