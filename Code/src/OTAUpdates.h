//Used for OTA updates
#include <ElegantOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

//Server object
AsyncWebServer server(81);

#ifndef OTA_UPDATES
#define OTA_UPDATES
class OTAUpdates
{
    public:

        const char *SSID = "AtntHotspot";
        const char *password = "Booooooo";
        void start()
        {
            WiFi.mode(WIFI_STA);
            WiFi.begin(SSID, password);
            // Wait for connection
            while (WiFi.status() != WL_CONNECTED) 
            {
                delay(500);
            }
            server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request-> send(200, "text/plain", "HDD Melty OTA Code Upload \nadd /update to the end of the URL to upload");
            });
            ElegantOTA.begin(&server);
            server.begin();
        }

        void loop()
        {
          ElegantOTA.loop();   
        }
};
#endif
