#include <Arduino.h>
#include <Wire.h>
#include<Adafruit_LIS331HH.h>
#include <Adafruit_Sensor.h>


#ifndef ACCEL_H
#define ACCEL_H

class Accel
{
    public:

    bool start( TwoWire &I2CBus, int address );

    float getAccel();

    //calibrates offsets for the acceleromiters
    void calibrate();


    private:
    //I2C pins
    int sclPin;
    int sdaPin;

    //I2C frequency
    int freq;

    //offsets to add to the acceleromiters
    float offsety = 0;
    float offsetx = 0;

    //acceleromiter objects
    TwoWire I2CBus = TwoWire(0);
    Adafruit_LIS331HH mpu = Adafruit_LIS331HH();

};

#endif