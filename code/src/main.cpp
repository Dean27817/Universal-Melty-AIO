#include "Arduino.h"
#include "motor.h"
#include "kinimatics.h"
#include "accel.h"
#include "OTAUpdates.h"
#include "led.h"
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>

//Xbox controller object
XboxSeriesXControllerESP32_asukiaaa::Core xbox;

//motor objects and variables
Motor motor1( 17, 0, 50, 0 );
Motor motor2( 18, 1, 50, 0 );
float motorSpeeds[2];

//acceleromiter objects and variables
Accel accel1;
float accel1Mag;
float currentAngle;
TwoWire I2CBus = TwoWire( 0 );
// persistent angle offset adjustable by D-pad left/right
float angleOffset = 0.0f;

//kinimatics object
kinimatics kine( 0.00313 );

//otaUpdate object
OTAUpdates OTA;
bool wifiEnabled = 0;

//heading LED objects
LED header1( 6 );

// lock-free sequence counter and minimal shared snapshot so neither task ever blocks
struct SharedState
{
    float currentAngle;
};

volatile uint32_t seqlock_counter = 0;
volatile SharedState sharedState = {0};

//the second loop, used for multithreading
void loop2( void *pvParameters );

void setup() 
{

    // Initialize hardware that requires Arduino core to be up
    kine.loadValues();

    I2CBus.begin( 2, 1 );
    accel1.start( I2CBus, 0x18 );

    // initialize objects that previously ran hardware calls in constructors
    motor1.begin();
    motor2.begin();
    header1.begin();
    kine.begin();

    xbox.begin();
    while( !xbox.isConnected() )
    {
        xbox.onLoop();
    }

    //start motors at 0 (motors armed after begin())
    motor1.setSpeed( 0 );
    motor2.setSpeed( 0 );
    delay( 2500 );

    header1.stopped();

    // start secondary loop after initialization to avoid pre-setup activity
    xTaskCreatePinnedToCore
    (
        loop2,
        "Loop2",
        8192,
        NULL,
        1,
        NULL,
        0
    );
}

void loop() 
{
    // update xbox/controller state on main thread
    xbox.onLoop();

    //variables to be used later
    float stickMag;
    float stickAngle;

    // Single-attempt seqlock read to get the latest angle snapshot.
    float localAngle = 0.0f;
    static float lastGoodAngle = 0.0f;

    uint32_t before = __atomic_load_n(&seqlock_counter, __ATOMIC_SEQ_CST);
    if ( before & 1 )
    {
        localAngle = lastGoodAngle; // writer in progress
    }
    else
    {
        localAngle = sharedState.currentAngle;
        uint32_t after = __atomic_load_n(&seqlock_counter, __ATOMIC_SEQ_CST);
        if ( before != after )
        {
            localAngle = lastGoodAngle; // inconsistent read
        }
        else
        {
            lastGoodAngle = localAngle;
        }
    }

    if( xbox.isConnected() )
    {
        float x = ((xbox.xboxNotif.joyRHori - 32767.5f) / 32767.5f);
        float y = ((xbox.xboxNotif.joyRVert - 32767.5f) / 32767.5f);

        //tank drive mode: when LB not held
        if( !xbox.xboxNotif.btnLB )
        {
            float left = -((xbox.xboxNotif.joyLVert - 32767.5f) / 32767.5f);
            float right = -((xbox.xboxNotif.joyRVert - 32767.5f) / 32767.5f);
            motor1.setSpeed( left * 0.3f );
            motor2.setSpeed( right * 0.3f );
            header1.stopped();
        }
        //melty mode
        else
        {
            //gets values from the stick to polar coordanates
            stickMag = hypot( x, y );
            stickAngle = atan2( y, x );


            // adjust forward position while spinning using left-stick horizontal
            float leftH = -((xbox.xboxNotif.joyLHori - 32767.5f) / 32767.5f);
            angleOffset += leftH * 0.0005f;

            float appliedAngle = localAngle + angleOffset;

            // normalize to [0, 2*PI)
            while ( appliedAngle >= (2.0f * PI) ) appliedAngle -= (2.0f * PI);
            while ( appliedAngle < 0.0f ) appliedAngle += (2.0f * PI);

            //find the speeds the motors need to be at any given instant
            kine.findMotorSpeeds
            (
                motorSpeeds,
                appliedAngle,
                stickMag,
                stickAngle,
                0.3f
            );

            //set the motors to the speeds calculated above
            motor1.setSpeed( motorSpeeds[0] );
            motor2.setSpeed( motorSpeeds[1] );

            if( xbox.xboxNotif.btnDirUp )
            {
                kine.increaseKnownRadius( 0.0000001f );
            }
            else if( xbox.xboxNotif.btnDirDown )
            {
                kine.decreaseKnownRadius( 0.0000001f );
            }

            // update heading LED in melty mode
            header1.onLoop( appliedAngle );
        }

        //saves the values that have been tuned
        if( xbox.xboxNotif.btnShare )
        {
            kine.saveValues();
        }

        //allows the wifi to be off at the beginning
        //makes the hotspot unneccacary for startup
        if( xbox.xboxNotif.btnXbox )
        {
            OTA.begin();
            wifiEnabled = 1;
        }
        if( wifiEnabled )
        {
            OTA.loop();
        }
    }

    //failsafes
    else
    {
        motor1.setSpeed( 0 );
        motor2.setSpeed( 0 );
        header1.error();
    }
}


void loop2( void *pvParameters )
{

    while( 1 )
    {
        //find the radial acceleration of both acceleromiters
        accel1Mag = accel1.getAccel();

        //find the current angle based on the two accelerations
        currentAngle = kine.findAngle( accel1Mag );

        // publish a minimal snapshot using a sequence counter (non-blocking)
        __atomic_add_fetch(&seqlock_counter, 1, __ATOMIC_SEQ_CST); // become odd -> writer in progress
        sharedState.currentAngle = currentAngle;
        __atomic_add_fetch(&seqlock_counter, 1, __ATOMIC_SEQ_CST); // become even -> writer done
        // yield to avoid starving main loop
        vTaskDelay( 1 );
    }

}
