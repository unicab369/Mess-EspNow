#define FORMAT_IF_FAILED true

enum FileAccessType {
    FS_READ, FS_WRITE, FS_APPEND,
};

class FileInterface {
    public:
        virtual File openFile(const char* path, FileAccessType type = FS_READ) { return File(); }
        virtual bool renameFile(const char *path1, const char *path2) { return false; }
        virtual bool deleteFile(const char *path)   { return false; }
        virtual bool makeDir(const char *path)      { return false; }
        virtual bool removeDir(const char *path)    { return false; }
        virtual bool exists(const char* path)       { return false; }

        const char* filePath(File *file) {
            #ifdef ESP32
                // return file->path();
                return file->name();    //???
            #else
                return file->fullName();
            #endif
        }
};

class Sto_Interface {
    protected:
        FileInterface *filesys;

    public:
        Sto_Interface(FileInterface *fs) {
            filesys = fs;
        }

        void readFile(const char *path) {
            Serial.printf("Reading file: %s\r\n", path);
            
            File file = filesys->openFile(path);
            bool check = !file || file.isDirectory();
            if (!check) { return; }

            Serial.println("- read from file:");
            while (file.available()) {
                Serial.write(file.read());
            }
            file.close();
        }

        bool writeFile(const char *path, const char *message) {
            Serial.printf("Writing file: %s\r\n", path);
            // File file = fs->open(path, FILE_WRITE);
            File file = filesys->openFile(path, FS_WRITE);
            if (!file) { return false; }

            bool write = file.print(message);
            Serial.println(write ? "- write success" : "- write failed");
            file.close();
            return write;
        }

        void makeFile(const char *path) {
            // AppPrint("[Sto]", String(__func__) + " " + String(path));

            if (filesys->exists(path)) return;
            if (strchr(path, '/')) {
                Serial.printf("Create missing folders of: %s\r\n", path);
                char *pathStr = strdup(path);
                if (pathStr) {
                    char *ptr = strchr(pathStr, '/');
                    while (ptr) {
                        *ptr = 0;
                        // fs->mkdir(pathStr);
                        filesys->makeDir(pathStr);
                        *ptr = '/';
                        ptr = strchr(ptr + 1, '/');
                    }
                }
                free(pathStr);
            }
        }

        void makeFile(const char *path, const char *message) {
            makeFile(path);
            if (message) writeFile(path, message);
        }

        void appendFile(const char *path, const char *message) {
            if (path[0] == '\0') {
                // AppPrint("[Sto]", "invalidPath");
                return;
            }

            // AppPrint("[Sto]", String(__func__) + " " + String(path));
            // File file = fs->open(path, FILE_APPEND);
            File file = filesys->openFile(path, FS_APPEND);
            if (!file) { return; }

            // AppPrint(file.print(message) ? "- append success" : "- append failed");
            file.print(message);
            file.close();
        }
        
        void deleteFile(const char *path) {
            filesys->deleteFile(path);

            char *pathStr = strdup(path);
            if (pathStr) {
                char *ptr = strrchr(pathStr, '/');
                if (ptr) {
                    Serial.printf("Removing all empty folders on path: %s\r\n", path);
                }
                while (ptr) {
                    *ptr = 0;
                    // fs->rmdir(pathStr);
                    filesys->removeDir(pathStr);
                    ptr = strrchr(pathStr, '/');
                }
                free(pathStr);
            }
        }

        void testFileIO(const char * path){
            File file = filesys->openFile(path);
            static uint8_t buf[512];
            size_t len = 0;
            uint32_t start = millis();
            uint32_t end = start;
            if(file) {
                len = file.size();
                size_t flen = len;
                start = millis();
                while(len){
                size_t toRead = len;
                if(toRead > 512){
                    toRead = 512;
                }
                file.read(buf, toRead);
                len -= toRead;
                }
                end = millis() - start;
                Serial.printf("%u bytes read for %u ms\n", flen, end);
                file.close();
            } 
            else {
                Serial.println("Failed to open file for reading");
            }

            // file = fs->open(path, FILE_WRITE);
            file = filesys->openFile(path, FS_WRITE);
            if (!file) { return; }

            size_t i;
            start = millis();
            for(i=0; i<2048; i++){
                file.write(buf, 512);
            }
            end = millis() - start;
            Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
            file.close();
        }

        void listDir(const char *dirname, std::function<void(File*)> forEach) {
            Serial.printf("Listing directory: %s\r\n", dirname);

            // File root = fs->open(dirname);
            File root = filesys->openFile(dirname);
            if (!root) { return; }

            if (!root.isDirectory()) {
                Serial.println(" - not a directory");
                return;
            }

            File file = root.openNextFile();
            while (file) {
                forEach(&file);
                file = root.openNextFile();
            }
        }
        
        void listDir(const char *dirname, uint8_t levels) {            
            listDir(dirname, [this, levels](File *file) {
                if (file->isDirectory()) {
                    Serial.print("  DIR : "); Serial.println(file->name());
                    if (levels) {
                        listDir(filesys->filePath(file), levels-1);
                        // listDir(file->path(), levels - 1);
                    }
                }
                else {
                    Serial.print("  FILE: "); Serial.print(file->name());
                    Serial.print("\tSIZE: "); Serial.println(file->size());
                }
            });
        }

        bool createDir(const char *path) { return filesys->makeDir(path); }
        bool removeDir(const char *path) { return  filesys->removeDir(path); }
        bool tryDeleteFile(const char *path) { return filesys->deleteFile(path); }
        bool renameFile(const char *path1, const char *path2) { return filesys->renameFile(path1, path2); }

        void test() {
            makeFile("/new1/new2/new3/hello3.txt", "Hello3");
            listDir("/", 3);
            deleteFile("/new1/new2/new3/hello3.txt");
            listDir("/", 3);
            createDir("/mydir");
            writeFile("/mydir/hello2.txt", "Hello2");
            listDir("/", 1);
            deleteFile("/mydir/hello2.txt");
            removeDir("/mydir");
            listDir("/", 1);
            writeFile("/hello.txt", "Hello ");
            appendFile("/hello.txt", "World!\r\n");
            readFile( "/hello.txt");
            renameFile("/hello.txt", "/foo.txt");
            readFile("/foo.txt");
            tryDeleteFile("/foo.txt");
            tryDeleteFile("/test.txt");
            Serial.println("Test complete\n");
        }
};