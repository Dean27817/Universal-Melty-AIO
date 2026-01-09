#include <Arduino.h>
#include "led.h"
#include <cmath>

LED::LED( int ledPinIn )
{
    //set object variables
    this -> ledPin = ledPinIn;
}

void LED::begin()
{
    pinMode( this -> ledPin, OUTPUT );
}


//the robot is stopped with no errors
void LED::stopped() 
{
    digitalWrite( this -> ledPin, HIGH );
}

void LED::onLoop( float angle )
{
    if( angle > ( 3 * PI ) / 2 && angle < ( 11 * PI ) / 6  )
    {
        digitalWrite( this -> ledPin, HIGH );
    }
    else
    {
        digitalWrite( this -> ledPin, LOW );
    }
}

//shows any errors found
void LED::error()
{
    //time since last check
    int deltaTime = millis() - this -> lastTime;

    //blinks the LED every half second
    if( deltaTime >= 500 )
    {
        //switches LED on or off
        if( !errorOn )
        {
            digitalWrite( this -> ledPin, HIGH );
        }
        else
        {
            digitalWrite( this -> ledPin, LOW );
        }

        //reverses LED on or off and sets current time
        errorOn = !errorOn;
        lastTime = millis();
    }
}


//resets all the variables
void LED::reset()
{
    currentLED = 0;
    lastTime = 0;
    increasing = 1;
}
