#include <LittleFS.h>

#include "Sto_Interface.h"
#include "SD.h"

class FS_Obj: public FileInterface {
    fs::FS *fs;

    public:
        FS_Obj(fs::FS *_fs) {
            fs = _fs;
        }

        File openFile(const char* path, FileAccessType type = FS_READ) override { 
            const char *mode = "r";     // default to read
            mode = (type == FS_WRITE) ? "w" : "a";
            File file = fs->open(path, mode);
            // AppPrint("[FS] Open", file ? "success" : "failed");
            return file;
        }

        bool renameFile(const char *path1, const char *path2) override {
            Serial.print("[FS] "); Serial.print(__func__);
            bool check = fs->rename(path1, path2);
            Serial.println(check ? " success" : " failed");
            return check;
        }

        bool deleteFile(const char *path) override {
            Serial.print("[FS] "); Serial.print(__func__); Serial.print(" "); Serial.print(path);
            bool check = fs->remove(path);
            Serial.println(check ? " success" : " failed");
            return check;
        }

        bool makeDir(const char *path) override {
            Serial.print("[FS] "); Serial.print(__func__); Serial.print(" "); Serial.print(path);
            bool check = fs->mkdir(path);
            Serial.println(check ? " success" : " failed");
            return check;
        }

        bool removeDir(const char *path) override {
            Serial.print("[FS] "); Serial.print(__func__); Serial.print(" "); Serial.print(path);
            bool check = fs->rmdir(path);
            Serial.println(check ? " success" : " failed");
            return check;
        }

        bool exists(const char* path) override { 
            return fs->exists(path);
        }
};


#ifdef ESP32
    class SD_Obj: public FS_Obj {
        public:
            SD_Obj(SDFS* sdfs): FS_Obj(sdfs) {}
    };
    
#else
    // TODO: TEST
    class SD_Obj: public FileInterface {
        SDClass *sd;

        public:
            SD_Obj(SDClass *_sd) {
                sd = _sd;
            }

            File openFile(const char* path, FileAccessType type = FS_READ) override {
                const char *mode = "r";     // default to read
                mode = (type == FS_WRITE) ? "w" : "a";
                File file = SD.open(path, mode);
                // AppPrint("[FS] Open", file ? "success" : "failed");
                return file;
            }

            bool renameFile(const char *path1, const char *path2) override {
                Serial.print("[SD] "); Serial.print(__func__); Serial.print(" "); Serial.print(path1);
                return sd->rename(path1, path2);
            }

            bool deleteFile(const char *path) override {
                Serial.print("[SD] "); Serial.print(__func__); Serial.print(" "); Serial.print(path);
                bool check = sd->remove(path);
                Serial.println(check ? " success" : " failed");
                return check;
            }

            bool makeDir(const char *path) override {
                Serial.print("[SD] "); Serial.print(__func__); Serial.print(" "); Serial.print(path);
                bool check = sd->mkdir(path);
                Serial.println(check ? " success" : " failed");
                return check;
            }

            bool removeDir(const char *path) override {
                Serial.print("[SD] "); Serial.print(__func__); Serial.print(" "); Serial.print(path);
                bool check = sd->mkdir(path);
                Serial.println(check ? " success" : " failed");
                return check;
            }

            bool exists(const char* path) override { 
                return sd->exists(path);
            }
    };
#endif


class Sto_LittleFS: public Sto_Interface {
    #ifdef ESP32
        FS_Obj obj = FS_Obj(&LittleFS);
    #else
        FS_Obj obj = FS_Obj(&LittleFS);
    #endif
    
    public:
        Sto_LittleFS() : Sto_Interface(&obj) {}

        void begin() {
            #ifdef ESP32
                // if (!LittleFS.begin(FORMAT_IF_FAILED)) {     // ESP32 ONLY
                    Serial.print("[LittleFS] Begin: "); Serial.println(LittleFS.begin() ? "success" : "failed");
                // }
            #else 
                Serial.print("[LittleFS] Begin: "); Serial.println(LittleFS.begin() ? "success" : "failed");
            #endif

        }
};

class Sto_SD: public Sto_Interface {
    SD_Obj obj = SD_Obj(&SD);
    bool ready = false;

    public:
        Sto_SD() : Sto_Interface(&obj) {}

        bool isReady() { return ready; }
        
        void begin(uint8_t cs) {
            ready = SD.begin(cs);
            // AppPrint("\n[SD] Begin", ready ? "success" : "failed");
        }

        uint64_t getCardSize() {
            #ifdef ESP32
                float sizeMb = SD.cardSize()/(1024*1024);
            #else
                // TODO
                float sizeMb = 0;
            #endif

            // Serial.println("[SD] size: " + String(sizeMb) + "Mb");
            // getFreeSpace();
            return sizeMb;
        }

        void getFreeSpace() {
            #ifdef ESP32
                uint32_t totalMb = SD.totalBytes()/(1024*1024);
                uint32_t usedMb = SD.usedBytes()/(1024*1024);
            #else
                // TODO
                uint32_t totalMb = 0;
                uint32_t usedMb = 0;
            #endif

            Serial.println("[SD] total: " + String(totalMb) + "Mb");
            Serial.println("[SD] used: " + String(usedMb) + "Mb");
        }
};