#include <Arduino.h>

#ifndef MOTOR_H
#define MOTOR_H

class Motor
{
    public:

    //initiates the LEDC channell
    Motor( int motorPinIn, int ledcChannelIn, int freq, bool reversed );

    // initialize hardware (call from setup())
    void begin();

    //sets the speed of the motor based on the percent speed passed in
    //passed a value between -1 and 1 for 100% backwards and 100% forwards
    void setSpeed( float speed );

    private:

    //the channel that the pin will be connected to
    int ledcChannelNum;
    //the pin that the motor is connected to
    int motorPin;
    int reversed = 1;
    int freq = 0;
};

#endif