#include <Arduino.h>
#include "motor.h"

//initialize the ledc channel for the motors
Motor::Motor( int motorPinIn, int ledcChannelIn, int freq, bool reversedIn )
{
    //sets the private values in the class
    this -> ledcChannelNum = ledcChannelIn;
    this -> motorPin = motorPinIn;

    // frequency and reversed are stored; actual hardware setup happens in begin()
    this -> freq = freq;
    if( reversedIn )
    {
        this -> reversed *= -1;
    }
}


//sets the speed of the motor based on the percent speed passed in
//passed a value between -1 and 1 for 100% backwards and 100% forwards
void Motor::setSpeed( float speed )
{
    //maps the percentage value to the one being used by the motor
    speed = constrain(speed, -1.0f, 1.0f);
    uint32_t ledcValue = 205 + (speed + 1.0f) * 0.5f * (410 - 205);

    //write to the motor
    ledcWrite(ledcChannelNum, ledcValue);

}

// initialize LEDC channel and attach pin (call from setup)
void Motor::begin()
{
    ledcSetup( ledcChannelNum, freq, 12 );
    ledcAttachPin( motorPin, ledcChannelNum );
}