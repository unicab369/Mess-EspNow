#include <Arduino.h>

#include "Net_Wifi.h"

#include "PacketModels.h"
#include "Net_EspNow.h"
#include "Serv_Tweet.h"

Net_Wifi wifi;
Net_EspNow espNow;
Serv_Tweet tweet;

int BROADCAST_CHANNEL = 10;

void configureESPNow() {
   wifi.setTxPower(0);
   wifi.startAP(true, BROADCAST_CHANNEL);
   tweet.setup(&espNow);
   espNow.setup(WiFi.channel());
   Serial.printf("\nWifiChannel = %u", WiFi.channel());

    espNow.callback = [&](ReceivedPacket* packet) {
        DataContent content = packet->dataPacket.content;
        char output[22];
        digitalWrite(2, !digitalRead(2));

        switch (packet->dataPacket.msgType) {
            case CMD_TRIGGER: {
                CommandItem item = content.commandItem;

                switch (item.cue) {
                    case TRIGGER_STARTUP: {
                        sprintf(output, "Recv Startup");
                        // device->addDisplayQueues("Recv Startup", 6);
                        break;
                    }
                    case TRIGGER_SINGLECLICK: {
                        sprintf(output, "Recv SingleClk: %lu", item.value);
                        // device->addDisplayQueues(output, 6);
                        // device->led.toggle();
                        break;
                    }
                    case TRIGGER_DOUBLECLICK: {
                        sprintf(output, "Recv DoubleClk: %lu", item.value);
                        // device->addDisplayQueues(output, 6);
                        // device->led.repeatPulses(1000);
                        break;  
                    }
                    case TRIGGER_PIR: {
                        sprintf(output, "Recv Pir: %lu", item.value);
                        // device->addDisplayQueues(output, 6);
                        break;
                    }
                    case TRIGGER_IR: {
                        sprintf(output, "Recv Ir: %lu", item.value);
                        // device->addDisplayQueues(output, 6);
                        break;
                    }
                    default: {
                        sprintf(output, "Recv Unknown");
                        break;
                    }
                }
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
                sprintf(output, "Recv CMD_PAIR");
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
    };
}

void setup() {
    Serial.begin(115200);
    pinMode(2, OUTPUT);
    pinMode(14, INPUT_PULLUP);

    configureESPNow();
}

void loop() {
    if (digitalRead(14)) {
        tweet.command.sendStartup();        delay(2000);
        tweet.command.sendSingleClick(1);   delay(2000);
        tweet.command.sendDoubleClick(2);   delay(2000);
        tweet.command.sendPir(3, 1);        delay(2000);
        tweet.command.sendIr(4444);         delay(2000);
        tweet.record.sendTempHumLux(1,2,3,4,5);     delay(2000);
        tweet.pairing.sendSyncMock();               delay(2000);
    } else {
        espNow.run();
    }

    // Serial.println("IM HERE");
    // digitalWrite(2, !digitalRead(2));
    // delay(2000);
}