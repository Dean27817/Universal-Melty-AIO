#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#ifndef LED_H
#define LED_H

class LED
{
    public:

    //begin communication with the LED strip
    LED( int ledPinIn );

    // initialize hardware (call from setup())
    void begin();

    //the robot is stopped with no errors
    void stopped();

    //melty heading
    void onLoop( float angle );

    //robot is stopped with errors
    void error();

    //sets all values back to defaulet
    //helps make the stopped lights look pretty
    void reset();

    private:

    //variables for the LEDC communication
    int ledPin;

    int currentLED = 0;
    int lastTime = 0;
    bool increasing = 1;
    bool errorOn = 0;
};

#endif