#include <Arduino.h>
#include "AppQueue.h"

#ifdef ESP32
    #include <esp_now.h>
#else
    #include <espnow.h>
#endif

byte _BROADCAST_ADDR[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

class Base_EspNow: public Interface_Net {            
	void addPeer(byte *peer, uint8_t channel) {
		#ifdef ESP32
			esp_now_peer_info_t peerInfo;
			
			memcpy(peerInfo.peer_addr, peer, 6);
			peerInfo.ifidx = WIFI_IF_STA;
			peerInfo.channel = channel;
			peerInfo.encrypt = false;
			// peerInfo.encrypt = useKey;
			if (esp_now_add_peer(&peerInfo) != 0) {
				Serial.println("[EspNow] Failed to add peer");
			}
		#else 
			esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
			esp_now_add_peer(peer, ESP_NOW_ROLE_SLAVE, channel, NULL, 0);
		#endif			
	}

    protected:
        bool isLoaded = false;
        
        void begin(int32_t channel, esp_now_recv_cb_t cb) {
            // esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
            // wifi_set_channel(channel);

            bool check = esp_now_init() == 0;
            Serial.printf("\n\n[EspN] Init = %u", isLoaded);

            esp_now_register_recv_cb(cb);
            addPeer(_BROADCAST_ADDR, channel);

            // // clear all the peers
            // #ifdef ESP32
            //         // turn on promiscuous to capture everything
            //     esp_wifi_set_promiscuous(true);
            //     esp_wifi_set_promiscuous_rx_cb(&onSniffCallback);

            //     esp_now_peer_info_t peerInfo;
            //     while (esp_now_fetch_peer(true, &peerInfo) == ESP_OK) {
            //         esp_now_del_peer(peerInfo.peer_addr);
            //     }
            // #else
            //     wifi_promiscuous_enable(1);
            //     wifi_set_promiscuous_rx_cb(onSniffCallback);
            // #endif

            isLoaded = true;
        }

    public: 
        uint8_t mac[6];

        uint8_t* getMac() override { return mac; }

        void sendData(DataPacket* data, size_t len) override {
            if (!isLoaded) { return; }
            esp_now_send(_BROADCAST_ADDR, (uint8_t*) data, len);

            #if DEBUG_ESPNOW
                Serial.println("\n[EspN] SendMessage = ");
                AppPrintHex(data, sizeof(DataPacket));
            #endif
        }

        // void sendCustomPacket() {
        //     // AppPrint("[EspN]", __func__);
        //     uint8_t addr1[6] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
        //     uint8_t addr2[6] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
        //     uint8_t addr3[6] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

        //     PacketInfo packet = PacketInfo::make(addr1, addr2, addr3);
        //     packet.data[0] = 0x77;
        //     AppPrint("MyPacket = ", packet.timestamp);
        //     AppPrintHex(&packet, sizeof(PacketInfo));

        //     #ifdef ESP32
        //         esp_wifi_80211_tx(WIFI_IF_AP, &packet, sizeof(packet) , false);
        //     #else
        //         uint8_t* pk = (uint8_t*)&packet;
        //         wifi_send_pkt_freedom(pk, sizeof(packet), false);
        //     #endif
        // }
};

#define MAX_MSG_QUEUE 5
// AppQueue<ReceivePacket2, MAX_MSG_QUEUE> msgQueue2;

#ifdef ESP32
    void receive_callback(const uint8_t *sender, const uint8_t *data, int data_len) {
        // ReceivePacket2 receiv_packet = ReceivePacket2::make(sender, data);
        // msgQueue2.sendQueue(&receiv_packet);
    }
#else
    #include <ArduinoQueue.h>
    ArduinoQueue<ReceivedPacket> msgQueue2(MAX_MSG_QUEUE);

    void receive_callback(uint8_t *sender, uint8_t *data, uint8_t len) {
        // Serial.println("RECEIVED MESSAGES");
        // Serial.print("\nReceiv from "); PrintBytes(sender, 6, ':');
        ReceivedPacket receiv_packet = ReceivedPacket::make(sender, data);
        msgQueue2.enqueue(receiv_packet);

        #if DEBUG_ESPNOW
            Serial.println("\n[EspN] ReceivMessage = ");
            AppPrintHex(&receiv_packet.dataPacket, sizeof(DataPacket));
        #endif
    }
#endif

class Net_EspNow: public Base_EspNow {
    public:
        // std::function<void(ReceivedPacket*)> callback = [](ReceivedPacket*){};

        void setup(uint32_t channel) {
            // AppPrintSeparator("[EspN]", __func__);
            begin(channel, receive_callback);
        }

        void run(std::function<void(ReceivedPacket*)> callback) {
            ReceivedPacket item;

            while (msgQueue2.isEmpty() == false) {
                item = msgQueue2.dequeue();
                callback(&item);
            }
        }
};
