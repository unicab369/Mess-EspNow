#include "Cue_Trigger.h"
#include "Mng_Network.h"
#include "Mng_StoSerial.h"

class Mng_Mess1 {
    SerialControl serial;
    Mng_Storage storage;

    Mng_Network network;
    public:
        void setup() {
            network.configureESPNow();
        }

        void run() {
            //! prevent task to handle serial print by other tasks
            serial.run([&](char* inputStr) {
                if (strcmp(inputStr, "ping") == 0)  {
                    Serial.println("What is thy bidding my Master?");
                }       
                else if (strcmp("wifiReset", inputStr) == 0) {
                    // onHandleResetWifi();
                }
                else if (strcmp("startAP", inputStr) == 0) {
                    // onHandleStartAP();
                }
                else if (strcmp("sendMock", inputStr) == 0) {
                    network.sendMock();
                }
                else if (strcmp("sendTest", inputStr) == 0) {
                    network.sendTest();
                }
                else if (storage.handleConsoleCmd(inputStr)) {
                } 
            });

            // if (!digitalRead(14)) {
            //     tweet.pairing.sendSyncMock(WiFi.channel());               
            //     delay(3000);
            // }

            network.run();
        }
};