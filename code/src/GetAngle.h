//used for communication with the sensor
//basic library for the arduino framework
#include <Arduino.h>
//used to help with the math in getting the angle from acceleration
#include <math.h>
//used for communication with the MPU
#include<Adafruit_LIS331HH.h>
#include <Adafruit_Sensor.h>
#define I2C_SDA 8
#define I2C_SCL 9
#define I2C_Freq 100000

int offset1 = 0;
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
    //also starts the I2C communicationvoid 
    void start()
	{
	    // Initialize I2C bus
	    I2CBus.begin(I2C_SDA, I2C_SCL, I2C_Freq);

	    bool mpu1_found = false;
	    bool mpu2_found = false;

	    // Try MPU1 at address 0x19 (accelerometer closer to center)
	    if (mpu1.begin_I2C(0x19, &I2CBus))
	    {
		mpu1_found = true;
		mpu1.setRange(LIS331HH_RANGE_24_G);
		mpu1.setDataRate(LIS331_DATARATE_1000_HZ);
	    }

	    // Try MPU2 at address 0x18 (accelerometer further from center)
	    if (mpu2.begin_I2C(0x18, &I2CBus))
	    {
		mpu2_found = true;
		mpu2.setRange(LIS331HH_RANGE_24_G);
		mpu2.setDataRate(LIS331_DATARATE_1000_HZ);
	    }

	    // If neither was found, hang here forever
	    if (!mpu1_found && !mpu2_found)
	    {
		while (true)
		{
		    delay(1000);
		}
	    }
	}



    //gets the acceleration from the sensors and saves it to global variables
    void getAccel()
    {
        //Get new sensor events with the readings
        sensors_event_t event1;
        mpu1.getEvent(&event1);

        sensors_event_t event2;
        mpu2.getEvent(&event2);

	//sets values
        mpu1mag, mpu2mag = event1.acceleration.y-offset1, event2.acceleration.y-offset2;
    }

    //it resets the gyro readings
    //this function took me hours to write
    //it may be my best written code ever
    //it is the pinical of all code ever written
    void ResetGyro()
    {
        currentRads = 0;
    }

    //averages a bunch of the readings togeather and will subtract them from the readings
    void Calibrate()
    {
        offset1 = 0;
        offset2 = 0;
        float tempOff1 = 0;
        float tempOff2 = 0;
        for(int i = 0; i<=1000; i++)
        {
	    getAccel();
            tempOff1 += mpu1mag;
            tempOff2 += mpu2mag;
        }
        offset1 = tempOff1/1000;
        offset2 = tempOff2/1000;

    }

    float getCurrentRads()
    {
	    return currentRads;
    }

    void loop()
    {
	    getAccel();
	    setRadius();
	    accellToRads();
    }


    private: 

    //the last time (in ms) that the angle was checked
    unsigned long lastTime = 0;
    //how far the robot has turned (in degrees)
    float currentRads = 0;

    //function that dynamically finds the radius using the two acceleromiters
    //accel1 should be the acceleromiter closer to the center, while accel2 is further
    float setRadius()
    {
	float radius = (mpu1mag*knownRadius)/(mpu1mag-mpu2mag);
	foundRadius = radius;
	return radius;
    }

    //finds the angular velocity by averaging the velocitys found by each acceleromiter
    float getAverageSpeed()
    {
	float angularVeloc1 = mpu1mag * foundRadius;
	float angularVeloc2 = mpu2mag * (foundRadius + knownRadius);

	float averageVeloc = (angularVeloc1 + angularVeloc2) / 2;
	return averageVeloc;

    }

    //takes in the data from the acceleromiter and returns the value of the degrees that the robot is at currently
    float accellToRads()
    {
        //gets the current uptime of the program in milliseconds
        unsigned long currentTime = millis();
        //finds the difference in seconds of the last check vs this one
        float deltaTime = (currentTime - lastTime) / 1000.0;

	//gets the speed of rotation and uses that along with time to find the displacment
	float angularVeloc = getAverageSpeed();
	currentRads += angularVeloc * deltaTime;

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
