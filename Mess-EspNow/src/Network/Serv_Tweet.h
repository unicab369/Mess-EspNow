class Interface_Net {
    public:
        virtual uint8_t* getMac() { 
            uint8_t mac[6] = {0};
            return mac; 
        }

        virtual void sendData(DataPacket* data, size_t len) {}
};

template <typename T, Type_Message command>
class Template_Tweet {
    protected:
        T item;

    public:    
        Interface_Net* interface;

        void __setup(Interface_Net* _interface) {
            interface = _interface;
        }

        void __sendMessage(uint8_t groupId = 0) {
            if (interface == nullptr) return;
            DataPacket packet = DataPacket::make(&item, command, groupId);
            interface->sendData(&packet, sizeof(DataPacket));
        }
};

class Tweet_Record: public Template_Tweet<RecordItem, CMD_POST> {
    public:
        void sendTempHumLux(float temp, float hum, float lux, float volt, float mA) {
            // AppPrint("\n[TweRecord]", __func__);
            item.update(temp, hum, lux, volt, mA);
            item.printData();
            __sendMessage();
        }
};

class Tweet_Command: public Template_Tweet<CommandItem, CMD_TRIGGER> {
    public:
        void sendStartup() {
            item.update(TRIGGER_STARTUP);
            __sendMessage();
        }

        void sendSingleClick(uint8_t pin) {
            item.update(TRIGGER_SINGLECLICK, pin);
            __sendMessage();
        }

        void sendDoubleClick(uint8_t pin) {
            item.update(TRIGGER_DOUBLECLICK, pin);
            __sendMessage();
        }

        void sendPir(uint8_t pin, bool state) {
            item.update(TRIGGER_PIR, pin, state);
            __sendMessage();
        }

        void sendIr(uint32_t irCode) {
            item.update(TRIGGER_IR, irCode);
            __sendMessage();
        }
};


class Tweet_Pair: public Template_Tweet<SyncItem, CMD_PAIR> {
    public:
        std::function<void(uint8_t)> *onReceiveBounce;

        void sendSyncMock(uint8_t channel) {
            item.cue = SYNC_MOCK;
            item.timeStamp = millis();
            item.srcChannel = channel;
            // item.setSource(interface->getMac());
            Serial.printf("\n** Send TimeStamp = %lu", item.timeStamp);
            __sendMessage(55);
        }

        void sendSyncBounce(uint8_t channel, uint32_t timeStamp) {
            item.cue = SYNC_BOUNCE;
            item.timeStamp = timeStamp;
            item.srcChannel = channel;
            __sendMessage(66);
        }
};

class Serv_Tweet {
    public:
        Tweet_Pair pairing;
        Tweet_Command command;
        // Tweet_Attendant attendant;
        Tweet_Record record;

        std::function<void()> setLadderIdCb = [&]() {
            // device->showLadderId();
        };

        void setup(Interface_Net *interface) {
            // attendant.setup(interface);
            command.__setup(interface);
            pairing.__setup(interface);
            record.__setup(interface);
            command.sendStartup();
        }
};