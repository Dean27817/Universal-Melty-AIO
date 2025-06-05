#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
WiFiServer server1(80);

#ifndef WEB_INTERFACE
#define WEB_INTERFACE

class WebInterface
{
public:
    // WiFi credentials
    const char *SSID = "AtntHotspot";
    const char *password = "Booooooo";

    String header;

    // Last known values to report
    float lastStickX = 0;
    float lastStickY = 0;
    float lastAngle = 0;
    float lastRadius = 0;
    bool lastSpin = 0;

    void start()
    {
        WiFi.begin(SSID, password);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
        }
        server1.begin();
    }

    void Talk(float stickX, float stickY, float angle, bool spin, float radius)
    {
        // Save the latest values to report
        lastStickX = stickX;
        lastStickY = stickY;
        lastAngle = angle;
        lastRadius =radius;
        lastSpin = spin;

        WiFiClient client = server1.available();

        if (client)
        {
            String currentLine = "";
            header = "";

            unsigned long timeout = millis() + 2000;

            while (client.connected() && millis() < timeout)
            {
                if (client.available())
                {
                    char c = client.read();
                    header += c;

                    if (c == '\n')
                    {
                        if (currentLine.length() == 0)
                        {
                            // Check if this is the AJAX data request
                            if (header.indexOf("GET /data") >= 0)
                            {
                                client.println("HTTP/1.1 200 OK");
                                client.println("Content-Type: text/plain");
                                client.println("Connection: close");
                                client.println();
                                client.print("Stick X: ");
                                client.print(lastStickX);
                                client.print(" | Stick Y: ");
                                client.print(lastStickY);
                                client.print(" | Spin Speed: ");
                                client.print(lastAngle);
                                client.print(" | Spin: ");
                                client.print(lastSpin);
                                client.print(" | Radius: ");
                                client.println(lastRadius);
                                client.println("Go to port 80 /update to upload code");
                                break;
                            }

                            // Serve main HTML page
                            client.println("HTTP/1.1 200 OK");
                            client.println("Content-type:text/html");
                            client.println("Connection: close");
                            client.println();

                            client.println("<!DOCTYPE html><html>");
                            client.println("<head><title>Meltybrain</title>");
                            client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
                            client.println("<style>body{font-family:sans-serif;text-align:center;}</style>");
                            client.println("</head>");
                            client.println("<body>");
                            client.println("<h1>Meltybrain Debug Interface</h1>");
                            client.println("<p id='data'>Loading...</p>");

                            client.println("<script>");
                            client.println("function updateData() {");
                            client.println(" fetch('/data')");
                            client.println("  .then(response => response.text())");
                            client.println("  .then(data => { document.getElementById('data').innerText = data; });");
                            client.println("}");
                            client.println("setInterval(updateData, 500);");
                            client.println("</script>");

                            client.println("</body></html>");
                            client.println();
                            break;
                        }
                        else
                        {
                            currentLine = "";
                        }
                    }
                    else if (c != '\r')
                    {
                        currentLine += c;
                    }
                }
            }

            delay(1);
            client.stop();
        }

    }
};

#endif