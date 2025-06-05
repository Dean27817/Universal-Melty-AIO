//library used for communication with the sensor
//basic library for the arduino framework
#include <Arduino.h>
//used to help with the math in getting the angle from acceleration
#include <math.h>
//used for communication with the MPU
#include <Adafruit_MPU6050.h>
#include<Adafruit_LIS331HH.h>
#include <Adafruit_Sensor.h>
#define I2C_SDA 2
#define I2C_SCL 1

//int offset1 = 0;
int offset2 = 0;

float mpu1mag;
float mpu2mag;

Adafruit_MPU6050 mpu1;
Adafruit_LIS331HH mpu2 = Adafruit_LIS331HH();



//checks if the class is already defined to avoid errors
#ifndef GET_ANGLE
#define GET_ANGLE
class GetAngle
{
    public:
    //meters
    double knownRadius = 0.0313;
    float foundRadius = 0;
    //initialized the class
    //sets the distance from the center to the sensor in mm
    //also starts the I2C communication
    void start()
    {
        Wire.begin(I2C_SDA, I2C_SCL);
       if (!mpu1.begin()) {
            while (1) {
                delay(10);
            }
        }
        //MPU1 setup
        mpu1.setAccelerometerRange(MPU6050_RANGE_8_G);
        mpu1.setGyroRange(MPU6050_RANGE_500_DEG);
        mpu1.setFilterBandwidth(MPU6050_BAND_5_HZ);

        //MPU2 setup
        if(!mpu2.begin_I2C())
        {
            while(1)
            {
                delay(10);
            }
        }
        mpu2.setRange(LIS331HH_RANGE_24_G);
        mpu2.setDataRate(LIS331_DATARATE_1000_HZ);
    }

    //recives the Rads that will get sent to the kinimatics function
    float GetRads()
    {
        //gets the current uptime of the program in milliseconds
        unsigned long currentTime = millis();
        //finds the difference in seconds of the last check vs this one
        float deltaTime = (currentTime - lastTime) / 1000.0;


        //gets the angle in radiens that it turned from the last check based on how long it has been spinning at a certain speed
        //then adds that value to the total distance turned
        currentRads += getSpeed() * deltaTime;
        //makes sure the value dosnt go above or bellow 360 degrees
        if (currentRads >= (2*PI))
        {
            currentRads -= (2*PI);
        }
        else if (currentRads < 0) 
        {
            currentRads += (2*PI);
        }
        lastTime = currentTime;
        return currentRads;
    }

    void ResetGyro()
    {
        currentRads = 0;
    }

    void Calibrate()
    {
        //offset1 = 0;
        offset2 = 0;
        float tempOff = 0;
        sensors_event_t a, g, temp, event;
        for(int i = 0; i<=1000; i++)
        {
            //mpu1.getEvent(&a, &g, &temp); 
            //mpu2.getEvent(&event);
            //offset1 += sqrt(pow(a.acceleration.x, 2)+pow(a.acceleration.y, 2));
            tempOff += getSpeed();
        }
        //offset1 = offset1/1000;
        offset2 = tempOff/1000;

    }

    float getSpeed()
    {
        /* Get new sensor events with the readings */
        sensors_event_t event;
        mpu2.getEvent(&event);


        float accel = abs(sqrt(pow(event.acceleration.y, 2)/*+pow(event.acceleration.x, 2)*/)-offset2);
        //finds the linear velocity both outer acceleromiters
        double velocity1 = sqrt((accel/(knownRadius)));
        //finds the angular velocity and averages both of them found with both accelleromiters seperatly
        return(velocity1/(2*PI*(knownRadius)));
        
    }


    private: 

    //the last time (in ms) that the angle was checked
    unsigned long lastTime = 0;
    //how far the robot has turned (in degrees)
    float currentRads = 0;

    //takes in the data from the acceleromiter and returns the value of the degrees that the robot is at currently
    float accellToRads()
    {
        //gets the current uptime of the program in milliseconds
        unsigned long currentTime = millis();
        //finds the difference in seconds of the last check vs this one
        float deltaTime = (currentTime - lastTime) / 1000.0;


        //gets the angle in radiens that it turned from the last check based on how long it has been spinning at a certain speed
        //then adds that value to the total distance turned
        currentRads += getSpeed() * deltaTime;
        //makes sure the value dosnt go above or bellow 360 degrees
        if (currentRads >= (2*PI))
        {
            currentRads -= (2*PI);
        }
        else if (currentRads < 0) 
        {
            currentRads += (2*PI);
        }
        lastTime = currentTime;
        return currentRads;
    }
};
#endif  