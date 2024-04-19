// #include <Queue.h>

// // template <typename T, int Size>
// template <int Size>
// class AppQueue {
// //     #ifdef ESP32
// //         QueueHandle_t queue;

// //         public:
// //             AppQueue() {
// //                 queue = xQueueCreate(Size, sizeof(T));
// //             }

// //             void sendQueue(T* item) {
// //                 if (xQueueSend(queue, item, 0) == pdTRUE) return;
// //                 AppPrint("[Queue] Full");
// //                 // T *itemToPop;          // Pop if full
// //                 // getQueue(itemToPop);
// //                 // xQueueSend(queue, item, 0);     // attempt to write again
// //             }

// //             bool getQueue(T *item) {
// //                 return xQueueReceive(queue, item, 0) == pdTRUE;
// //             }
// //     #else
//         #include <ArduinoQueue.h>

//         ArduinoQueue<T> queue(Size);
        
//         public:
//             void sendQueue(T *item) {
//                 queue.enqueue(*item);
//             }

//             bool getQueue(T *item) {
//                 if (queue.isEmpty()) return false;
//                 T first = queue.front();
//                 memcpy(item, &first, sizeof(T));
//                 // AppPrint("\nFound Queue");
//                 // AppPrint("value", String(item->data));
//                 queue.dequeue();
//                 return true;
//             }
// //     #endif
// };
