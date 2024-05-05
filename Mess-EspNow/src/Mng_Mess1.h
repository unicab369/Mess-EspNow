#include "PublicFunctions.h"
#include "Mng_Network.h"
#include "Mng_StoSerial.h"

class Mng_Mess1 {
    SerialControl serial;
    Mng_Storage storage;

    Mng_Network network;
    ControlTimer cTimer;

    unsigned long scanningTimeRef;
    int scanningChannel = 0;

    void startScanning() {
        scanningChannel = 1;
        scanningTimeRef = millis();
    }

    public:
        void setup() {
            network.configureESPNow(10);
        }

        void run() {
            cTimer.run([&]() {
                if (cTimer.isSecondInterval(1)) {
                    Serial.printf("\n*cycleCount = %lu, maxCycleTime = %lu", cTimer.cycleCount, cTimer.maxCycleTime);
                }

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
                    else if(strcmp("scanChannel", inputStr) == 0) {

                    }
                    else if (storage.handleConsoleCmd(inputStr)) {
                    } 
                });

                network.run();
            });

            // if (!digitalRead(14)) {
            //     tweet.pairing.sendSyncMock(WiFi.channel());               
            //     delay(3000);
            // }
        }
};