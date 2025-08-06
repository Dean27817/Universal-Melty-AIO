#include <Arduino.h>

#ifndef HEADER_LED
#define HEADER_LED 
class HeaderLED
{
    bool on = false;
    int LEDPin;
    unsigned long time = 0;
    public:
    HeaderLED(int Pin)
    {
        //sets the pin that the LED is attatched to and sets that pin to an output
        LEDPin = Pin;
        pinMode(LEDPin, OUTPUT);
    }

    //turns on the LED if the Robot is in the "forward" position
    void checkLED(float rads)
    {
        if (rads > PI/2 && rads < PI)
        {
            digitalWrite(LEDPin, HIGH);
        }
        else
        {
            digitalWrite(LEDPin, LOW);
        }
    }

    //if the robot is not spinning make sure the LED is on
    void RobotStopped()
    {
        digitalWrite(LEDPin, HIGH);
    }

    void blink()
    {
        unsigned long currentTime = millis();
        float deltaTime = (currentTime - time) / 1000.0;
        time = currentTime;
        if(deltaTime >= 0.7)
        {
            on = !on;
        }
        if(on)
        {
            digitalWrite(LEDPin, HIGH);
        }
        else 
        {
            digitalWrite(LEDPin, LOW);
        }
    }
};
#endif