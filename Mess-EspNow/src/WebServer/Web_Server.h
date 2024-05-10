#ifdef ESP32
    #include <WebServer.h>
    #include <HTTPClient.h>

#else
    #include <ESP8266WebServer.h>
    #include <ESP8266HTTPClient.h>

    class WebServer: public ESP8266WebServer {
        public:
            WebServer(int port) : ESP8266WebServer(port) {}
    };
#endif

WebServer webServer(80);

class Web_Server {
    // https://shepherdingelectrons.blogspot.com/2019/04/esp8266-as-spiffs-http-server.html
    String getContentType(String filename) {
        if (filename.endsWith(".html"))     return "text/html";
        else if (filename.endsWith(".css")) return "text/css";
        else if (filename.endsWith(".js"))  return "application/javascript";
        else if (filename.endsWith(".ico")) return "image/x-icon";
        return "text/plain";
    }

    //! pio run --target buildfs
    //! pio run --target uploadfs
    
    bool handleFileRead(String path) {
        Serial.println("\nhandleFileRead: " + path);
        if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
        String contentType = getContentType(path);            // Get the MIME type
        
        File file = onGetFile(path.c_str());
        if (file) {
            unsigned long ref = millis();
            size_t sent = webServer.streamFile(file, contentType);      // send to the client
            file.close();                                               // close the file
            Serial.printf("\nFileSize=%zu, ReadTime=%lu", file.size(), millis()-ref);
            return true;
        }
        
        return false;                                         // If the file doesn't exist, return false
    }

    public:
         std::function<File(const char*)> onGetFile = [](const char* path) { return File(); };

        void setup() {
            webServer.onNotFound([&]() {                              // If the client requests any URI
                if (!handleFileRead(webServer.uri()))                  // send it if it exists
                    webServer.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
            });

            webServer.begin();
        }

        void run() {
            webServer.handleClient();
        }
};