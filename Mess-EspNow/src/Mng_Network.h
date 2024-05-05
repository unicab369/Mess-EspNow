#include "Network/Net_Wifi.h"

#include "Network/PacketModels.h"
#include "Network/Serv_Tweet.h"
#include "Network/Net_EspNow.h"

enum Network_State {
    NETWORK_FAILED,
    NETWORK_DEFAULT,
    NETWORK_SCANNING
};

class Mng_Network {
    Net_Wifi wifi;
    Net_EspNow espNow;
    Serv_Tweet tweet;
    Network_State state = NETWORK_DEFAULT;

    void _handleCmdTrigger(CommandItem *item) {
        char output[22];
        switch (item->cue) {
            case TRIGGER_STARTUP: {
                sprintf(output, "\nRecv Startup");
                // device->addDisplayQueues("Recv Startup", 6);
                break;
            }
            case TRIGGER_SINGLECLICK: {
                sprintf(output, "\nRecv SingleClk: %lu", item->value);
                // device->addDisplayQueues(output, 6);
                // device->led.toggle();
                break;
            }
            case TRIGGER_DOUBLECLICK: {
                sprintf(output, "\nRecv DoubleClk: %lu", item->value);
                // device->addDisplayQueues(output, 6);
                // device->led.repeatPulses(1000);
                break;  
            }
            case TRIGGER_PIR: {
                sprintf(output, "\nRecv Pir: %lu", item->value);
                // device->addDisplayQueues(output, 6);
                break;
            }
            case TRIGGER_IR: {
                sprintf(output, "\nRecv Ir: %lu", item->value);
                // device->addDisplayQueues(output, 6);
                break;
            }
            default: {
                sprintf(output, "\nRecv Unknown");
                break;
            }
        }
    }

    void _handleCmdPair(SyncItem *item) {
        switch (item->cue) {
            case SYNC_MOCK: {
                //! Slave received
                Serial.println("\n[Tweet_Pair] SYNC_MOCK received");
                Serial.printf("\n***Recv Timestamp == %lu", item->timeStamp);
                tweet.pairing.sendSyncBounce(WiFi.channel(), item->timeStamp);
                break;
            }
            case SYNC_BOUNCE: {
                //! Master received
                Serial.println("\n[Tweet_Pair] SYNC_BOUNCE received");
                Serial.printf("\n*** BouncTime == %lu", item->timeStamp);
                Serial.printf("\n current Time = %lu", millis());
                
                uint32_t transTime = item->getTransmitTime();
                Serial.printf("\nTransTime = %lu", transTime);
                state = NETWORK_DEFAULT;    //! Stop Scanning
                break;
            }
            default: {
                Serial.println("\n[Tweet_Pair] Unknown");
                break;
            }
        }
    }

    int scanningChannel = 1;

    public:
        void scanningStart() {
            scanningChannel = 1;
            state = NETWORK_SCANNING;
        }

        void scanningTick() {
            if (state != NETWORK_SCANNING) return;

            if (scanningChannel>12) {
                state = NETWORK_DEFAULT;
                return;
            }

            configureESPNow(scanningChannel);
            sendMock();
            scanningChannel++;
        }

        void configureESPNow(int channel) {
            wifi.setTxPower(0);
            wifi.startAP(false, channel);
            tweet.setup(&espNow);

            espNow.setup(WiFi.channel());
            Serial.printf("\n[Net] WifiChannel = %u", WiFi.channel());
        }

        void sendMock() {
            tweet.pairing.sendSyncMock(WiFi.channel());
        }

        void sendTest() {
            tweet.command.sendStartup();        delay(500);
            tweet.command.sendSingleClick(1);   delay(500);
            tweet.command.sendDoubleClick(2);   delay(500);
            tweet.command.sendPir(3, 1);        delay(500);
            tweet.command.sendIr(4444);         delay(500);
            tweet.record.sendTempHumLux(1,2,3,4,5);     delay(500);
            tweet.pairing.sendSyncMock(1); delay(500);
        }

        void run() {
            espNow.run([&](ReceivedPacket* packet) {
                DataContent content = packet->dataPacket.content;
                char output[22];
                digitalWrite(2, !digitalRead(2));

                switch (packet->dataPacket.msgType) {
                    case CMD_TRIGGER: {
                        _handleCmdTrigger(&content.commandItem);
                        // addPlotterQueue(packet); break;
                        break;
                    }
                    case CMD_POST: {
                        sprintf(output, "Recv CMD_POST");
                        content.recordItem.printData();

                        // device->addDisplayQueues("Recv CMD_POST: ", 6);
                        // addPlotterQueue(packet); break;
                        break;
                    }
                    case CMD_PAIR: {
                        _handleCmdPair(&content.syncItem);
                        // device->addDisplayQueues("Recv CMD_PAIR: ", 6);
                        break;
                    }
                    case CMD_ATTENDANT: {
                        sprintf(output, "Recv CMD_ATTENDANT");
                        // device->addDisplayQueues("Recv CMD_ATTENDANT: ", 6);
                        break;
                    }
                }

                Serial.println(output);
            });
        }
};