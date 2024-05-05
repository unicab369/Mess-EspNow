#ifdef ESP32
    // RTC_DATA_ATTR char MY_SSID[33] = "";
    // RTC_DATA_ATTR char MY_PASSW[64] = "";
#else
extern "C" {
    #include "user_interface.h"
}
#endif

class Sto_RTC {
    public:
        void clear() {
            // TODO
        }

        void writeValue(uint32_t des_addr, void *src, uint32_t size) {
            #ifndef ESP32
            system_rtc_mem_write(des_addr, src, size);
            yield(); 
            #endif      
        }

        void readValue(uint32_t des_addr, void *target, uint32_t size) {
            #ifndef ESP32
            system_rtc_mem_read(des_addr, target, size);
            yield();
            #endif
        }
};