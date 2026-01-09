#include <Arduino.h>
#include "accel.h"
#include <Wire.h>
#include<Adafruit_LIS331HH.h>
#include <Adafruit_Sensor.h>

//starts the I2C with the acceleromiter
bool Accel::start( TwoWire &I2CBus, int address )
{
    //checks for connection and sets range
    if( this -> mpu.begin_I2C( address, &I2CBus ) )
    { 
        this -> mpu.setRange( LIS331HH_RANGE_24_G );
		this -> mpu.setDataRate(LIS331_DATARATE_1000_HZ);
        return 1;
    }

    //returns failure
    else
    {
        return 0;
    }
}


//gets the acceleration
float Accel::getAccel()
{
    float accelMag;
    sensors_event_t event;
    mpu.getEvent( &event );

    accelMag = 
        hypot( 
             event.acceleration.x - offsetx,
            event.acceleration.y - offsety
        );

    return accelMag;

}


//calibrates offsets for the acceleromiters
void Accel::calibrate()
{

}