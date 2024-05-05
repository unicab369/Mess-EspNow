#include "Network/Net_Wifi.h"

#include "Network/PacketModels.h"
#include "Network/Serv_Tweet.h"
#include "Network/Net_EspNow.h"

class Mng_Network {
    Net_Wifi wifi;
    Net_EspNow espNow;
    Serv_Tweet tweet;

    int BROADCAST_CHANNEL = 10;

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
                break;
            }
            default: {
                Serial.println("\n[Tweet_Pair] Unknown");
                break;
            }
        }
    }

    void _handleCmdTrigger(CommandItem *item) {
        char output[22];
        switch (item->cue) {
            case TRIGGER_STARTUP: {
                sprintf(output, "Recv Startup");
                // device->addDisplayQueues("Recv Startup", 6);
                break;
            }
            case TRIGGER_SINGLECLICK: {
                sprintf(output, "Recv SingleClk: %lu", item->value);
                // device->addDisplayQueues(output, 6);
                // device->led.toggle();
                break;
            }
            case TRIGGER_DOUBLECLICK: {
                sprintf(output, "Recv DoubleClk: %lu", item->value);
                // device->addDisplayQueues(output, 6);
                // device->led.repeatPulses(1000);
                break;  
            }
            case TRIGGER_PIR: {
                sprintf(output, "Recv Pir: %lu", item->value);
                // device->addDisplayQueues(output, 6);
                break;
            }
            case TRIGGER_IR: {
                sprintf(output, "Recv Ir: %lu", item->value);
                // device->addDisplayQueues(output, 6);
                break;
            }
            default: {
                sprintf(output, "Recv Unknown");
                break;
            }
        }
    }

    public:
        void configureESPNow() {
            wifi.setTxPower(0);
            wifi.startAP(true, BROADCAST_CHANNEL);
            tweet.setup(&espNow);

            espNow.setup(WiFi.channel());
            Serial.printf("\nWifiChannel = %u", WiFi.channel());
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