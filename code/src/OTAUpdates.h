//Used for OTA updates
#include <ElegantOTA.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
// Try FFat (FAT on flash) first since board partition table may use ffat
#include <FFat.h>
// fallback to LittleFS
#include <LittleFS.h>
#include <esp_partition.h>

//Server object
AsyncWebServer server(81);

#ifndef OTA_UPDATES
#define OTA_UPDATES
class OTAUpdates
{
    public:

        String SSID;
        String password;
        void begin()
        {


            int mountedFS = 0; // 1 = FFat, 2 = LittleFS

            bool ok = FFat.begin();

            if ( ! ok )
            {
                FFat.format();
                ok = FFat.begin();
            }

            if ( ok )
            {
                mountedFS = 1;
            }

            // fallback to LittleFS
            if ( ! mountedFS )
            {
                ok = LittleFS.begin();
                if ( ! ok )
                {
                    LittleFS.format();
                    ok = LittleFS.begin();
                }
                if ( ok )
                {
                    mountedFS = 2;
                }
            }

            if ( ! mountedFS )
            {
                esp_partition_iterator_t it = esp_partition_find( ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL );
                if ( it == NULL )
                {
                }
                else
                {
                    do
                    {
                        const esp_partition_t* p = esp_partition_get( it );
                        it = esp_partition_next( it );
                    }
                    while ( it );
                    esp_partition_iterator_release( it );
                }

                while ( true )
                {
                    delay( 1000 );
                }
            }

            // Print stats for whichever FS mounted
            if ( mountedFS == 2 )
            {
                size_t total = LittleFS.totalBytes();
                size_t used = LittleFS.usedBytes();
            }
            else
            {
                // FFat doesn't provide totalBytes() in same API; check via statvfs not available — just report existence
            }

            bool fileExists = false;
            if ( mountedFS == 2 ) fileExists = LittleFS.exists( "/passwords.txt" );
            else fileExists = FFat.exists( "/passwords.txt" );


            // If the mounted FS doesn't have the file, try the other FS (handles uploader/partition mismatch)
            if ( ! fileExists )
            {
                if ( mountedFS == 2 )
                {
                    // currently LittleFS mounted — try FFat
                    if ( FFat.begin() )
                    {
                        if ( FFat.exists( "/passwords.txt" ) )
                        {
                            mountedFS = 1;
                            fileExists = true;
                        }
                    }
                }
                else
                {
                    // currently FFat mounted — try LittleFS
                    if ( LittleFS.begin() )
                    {
                        if ( LittleFS.exists( "/passwords.txt" ) )
                        {
                            mountedFS = 2;
                            fileExists = true;
                        }
                    }
                }
            }

            // open credentials file (hang if missing)
            File passwordFile;
            if ( mountedFS == 2 )
            {
                passwordFile = LittleFS.open( "/passwords.txt", FILE_READ );
            }
            else
            {
                passwordFile = FFat.open( "/passwords.txt", FILE_READ );
            }

            if ( ! passwordFile )
            {
                while ( true )
                {
                    delay( 10 );
                }
            }

            // read lines and trim newline/whitespace
            SSID = passwordFile.readStringUntil( '\n' );
            SSID.trim();
            password = passwordFile.readStringUntil( '\n' );
            password.trim();
            passwordFile.close();

            WiFi.mode( WIFI_STA );
            WiFi.begin( SSID.c_str(), password.c_str() );

            // Wait for connection (hang until connected)
            while ( WiFi.status() != WL_CONNECTED )
            {
                delay( 500 );
            }

            server.on( "/", HTTP_GET, []( AsyncWebServerRequest * request )
            {
                request->send( 200, "text/plain", "HDD Melty OTA Code Upload \nadd /update to the end of the URL to upload" );
            } );

            ElegantOTA.begin( & server );
            server.begin();
        }

        void loop()
        {
          ElegantOTA.loop();   
        }
};
#endif
