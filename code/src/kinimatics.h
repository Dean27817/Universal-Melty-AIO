#include <Arduino.h>
#include "accel.h"
#include <cmath>
// Try FFat (FAT on flash) first since board partition table may use ffat
#include <FFat.h>
// fallback to LittleFS
#include <LittleFS.h>
#include <esp_partition.h>


#ifndef KINIMATICS_H
#define KINIMATICS_H

class kinimatics
{
    public:

    kinimatics( float knownRadiusIn );
    kinimatics( float knownRadiusIn, float foundRadiusIn );

    ///////////////////////////////////////////////////////////////////////////////////
    //////////////////                  Two acceleromiter variation     ///////////////
    ///////////////////////////////////////////////////////////////////////////////////

    //finds the angle based on the two acceleration values in g
    float findAngle( float accel1, float accel2 );

    //gets the current rotational velocity of the robot based on the radial acceleration and radius
    float findCurrentSpeed( float accel1, float accel2 );

    //calibrate the unknown radius based on the two acceleromiters
    //this will only really work if the two are on the same side of the bot
    void calibrateRadius( float acel1, float acel2 );



    ///////////////////////////////////////////////////////////////////////////////////
    //////////////////                  One acceleromiter variation     ///////////////
    ///////////////////////////////////////////////////////////////////////////////////

    //finds the angle based on the two acceleration values in g
    float findAngle( float accel1 );

    //gets the current rotational velocity of the robot based on the radial acceleration and radius
    float findCurrentSpeed( float accel1 );



    //finds the speed of the motor based on many factors
    //returns values between -1 and 1
        void findMotorSpeeds
                ( float motorSpeeds[2], float currentRads, float translationSpeed,
                    float translationRads, float spinSpeed 
                    );

    //allows tuning of the known radius
    void increaseKnownRadius( float increaseBy );
    void decreaseKnownRadius( float decreaseBy );

    //saves all the current melty tuning variables to a file
    //also prints them out
    bool saveValues();

    //loads tuned values from file if present; returns true if loaded
    bool loadValues();

    //prints the kinimatics values to the screen
    //allows for tuning on the robot and applying to the config file
    void printValues();

    // initialize runtime state (call from setup after Arduino initialized)
    void begin();

    private:

    //the two radii
    //used to find angular velocity from acceleration
    float knownRadius;
    float foundRadius;

    // which filesystem is mounted: 0 = none, 1 = FFat, 2 = LittleFS
    int mountedFS = 0;

    bool openFile();
    bool writeFile();

    //time variables
    float deltaTime = 0;
    float lastTime = 0;

    //angle
    float currentAngle = 0;
};

#endif
