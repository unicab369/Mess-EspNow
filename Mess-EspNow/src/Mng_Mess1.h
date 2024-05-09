#include "PublicFunctions.h"
#include "Mng_Network.h"
#include "Mng_StoSerial.h"

class Mng_Mess1 {
    SerialControl serial;
    Mng_Storage storage;

    Mng_Network network;
    ControlTimer cTimer;
    Web_Server wServer;

    Sto_LittleFS littleFS;

    void printLog(const char* output) {
        Serial.printf("\n[Mng_Mess1] %s", output);
    }

    void startWifi() {
        printLog("startWifi");
        Data_Cred cred = storage.stoCred.value;
        cred.printData();  
        network.startSTA(cred.ssid, cred.password);
        // network.configureESPNow(10);
    }

    public:
        void setup() {
            storage.setupStorage();
            startWifi();

            littleFS.begin();

            wServer.onGetFile = [&](const char* path) {
                // File file = littleFS.obj.openFile("/pages/page1.html");
                File file = littleFS.obj.openFile(path);
                //! FOR TESTING ONLY DON'T USE
                // if (file) {
                //     while (file.available()) {
                //         Serial.print(file.read());
                //     }
                //     file.close();
                // }
                return file;
            };

            wServer.setup();

            network.scanningComplete = [&](int channel) {
                Serial.println("\nIM HERE 3333");
            };
        }

        void run() {
            cTimer.run([&]() {
                if (cTimer.isSecondInterval(1)) {
                    if (storage.stoSettings.value.logCycle == true) {
                        Serial.printf("\n*cycleCount = %lu, maxCycleTime = %lu", cTimer.cycleCount, cTimer.maxCycleTime);
                    }
                    
                    network.networkTick([]() {
                        digitalWrite(2, !digitalRead(2));
                    });
                }

                serial.run([&](char* inputStr) {
                    if (strcmp(inputStr, "ping") == 0)  {
                        Serial.println("What is thy bidding my Master?");
                    }       
                    else if (strcmp("wifiReset", inputStr) == 0) {
                        startWifi();
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
                        network.startScanning();
                    }
                    else if (storage.handleConsoleCmd(inputStr)) {
                    } 
                });

                network.run();
                wServer.run();
            });

            // if (!digitalRead(14)) {
            //     tweet.pairing.sendSyncMock(WiFi.channel());               
            //     delay(3000);
            // }
        }
};