//used for communication with the sensor
//basic library for the arduino framework
#include <Arduino.h>
//used to help with the math in getting the angle from acceleration
#include <math.h>
//used for communication with the MPU
#include<Adafruit_LIS331HH.h>
#include <Adafruit_Sensor.h>
#define I2C_SDA 35
#define I2C_SCL 36
#define I2C_Freq 100000

//int offset1 = 0;
int offset2 = 0;

float mpu1mag;
float mpu2mag;

TwoWire I2CBus = TwoWire(0);
Adafruit_LIS331HH mpu1 = Adafruit_LIS331HH();
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
        I2CBus.begin(I2C_SDA, I2C_SCL, I2C_Freq);
	int Address1 = 0;
	byte busStatus;
	for (int i2cAddress = 0x00; i2cAddress < 0x80; i2cAddress++)
	  {
	    Wire.beginTransmission(i2cAddress);
	    busStatus = Wire.endTransmission();
	    if (busStatus == 0x00)
	    {
	      Serial.print("I2C Device found at address: 0x");
	      Serial.println(i2cAddress, HEX);
	      Address1 = i2cAddress;
	    }

	    else
	    {
	      Serial.print("I2C Device not found at address: 0x");
	      Serial.println(i2cAddress, HEX);
	    }
	  }
	//both sensors wont read. I am thinking this may be a hardware issue, as it is happening with both and I have tried using wire and twowire
        //MPU1 setup
	//this sensor is also having issues
        if(!mpu1.begin_I2C(Address1))
        {
            while(1)
            {
                delay(10);
            }
        }
	/*
        mpu1.setRange(LIS331HH_RANGE_24_G);
        mpu1.setDataRate(LIS331_DATARATE_1000_HZ);

        //MPU2 setup
	//we know that this sensor has an issue
	//this is the sensor closer to the ESP32
        if(!mpu2.begin_I2C(0x18))
        {
            while(1)
            {
                delay(10);
            }
        }
        mpu2.setRange(LIS331HH_RANGE_24_G);
        mpu2.setDataRate(LIS331_DATARATE_1000_HZ);
	*/
	
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
        //mpu2.getEvent(&event);


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
