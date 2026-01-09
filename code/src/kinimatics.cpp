
#include <Arduino.h>
#include "kinimatics.h"
#include "accel.h"
#include <cmath>
// Try FFat (FAT on flash) first since board partition table may use ffat
#include <FFat.h>
// fallback to LittleFS
#include <LittleFS.h>
#include <esp_partition.h>


//constructors for one and two acceleromiters
// Single-argument constructor: set `knownRadius` from caller.
// Then attempt to load persisted tuning values from filesystem; if
// a saved config exists it will override this constructor value.
kinimatics::kinimatics( float knownRadiusIn )  
{
    this -> knownRadius = knownRadiusIn;
    // leave timing uninitialized until begin() is called after Arduino init
    this->lastTime = 0.0f;
}


// Two-argument constructor: set both `knownRadius` and `foundRadius`.
// Then attempt to load persisted tuning values which will replace
// these defaults if a config file is present on the device.
kinimatics::kinimatics( float knownRadiusIn, float foundRadiusIn )
{
    this -> knownRadius = knownRadiusIn;
    this -> foundRadius = foundRadiusIn;
    // leave timing uninitialized until begin() is called after Arduino init
    this->lastTime = 0.0f;
}

// initialize runtime timing state; must be called from setup()
void kinimatics::begin()
{
    this->lastTime = millis() / 1000.0f;
}


//finds the current angular velocity wihth one acceleromiter
//measured in mps
float kinimatics::findCurrentSpeed( float accel1 )
{
    // guard against invalid inputs
    if ( accel1 <= 0.0f ) return 0.0f;
    if ( this->knownRadius <= 0.0f ) return 0.0f;

    float mpsAccel = accel1;
    float omega = sqrt( mpsAccel / this->knownRadius );
    if ( isnan( omega ) ) return 0.0f;
    return omega;
}

//finds the current angular velocity wihth two acceleromiters
//measured in mps
float kinimatics::findCurrentSpeed( float accel1, float accel2 )
{
    // guard against invalid inputs
    if ( this->knownRadius <= 0.0f ) return 0.0f;
    if ( this->foundRadius + this->knownRadius <= 0.0f ) return 0.0f;

    float mpsAccel1 = accel1;
    float mpsAccel2 = accel2;

    float angveloc1 = 0.0f;
    float angveloc2 = 0.0f;

    if ( mpsAccel1 > 0.0f ) angveloc1 = sqrt( mpsAccel1 / this->knownRadius );
    if ( mpsAccel2 > 0.0f ) angveloc2 = sqrt( mpsAccel2 / ( this->knownRadius + this->foundRadius ) );

    float omega = ( angveloc1 + angveloc2 ) / 2.0f;
    if ( isnan( omega ) ) return 0.0f;
    return omega;
}


//finds the current angle wihth one acceleromiter
//measured in g
float kinimatics::findAngle( float accel1 )
{
    // find the change in time since last check (use float seconds)
    float now = millis() / 1000.0f;
    // protect first-run where lastTime may be zero/uninitialized
    if ( this->lastTime <= 0.0f )
    {
        this->deltaTime = 0.0f;
    }
    else
    {
        this->deltaTime = now - this->lastTime;
    }
    this->lastTime = now;

    // find the angle through integration of angular velocity
    float currentSpeed = findCurrentSpeed( accel1 );
    this->currentAngle += currentSpeed * this->deltaTime;

    // ensure the angle stays in [0, 2*PI)
    while (this->currentAngle >= (2.0f * PI))
    {
        this->currentAngle -= (2.0f * PI);
    }
    while (this->currentAngle < 0.0f)
    {
        this->currentAngle += (2.0f * PI);
    }

    return this->currentAngle;
}


//finds the current angle wihth two acceleromiters
//measured in g
float kinimatics::findAngle( float accel1, float accel2 )
{
    // find the change in time since last check (use float seconds)
    float now = millis() / 1000.0f;
    // protect first-run where lastTime may be zero/uninitialized
    if ( this->lastTime <= 0.0f )
    {
        this->deltaTime = 0.0f;
    }
    else
    {
        this->deltaTime = now - this->lastTime;
    }
    this->lastTime = now;

    // find the angle through integration of angular velocity
    float currentSpeed = findCurrentSpeed( accel1, accel2 );
    this->currentAngle += currentSpeed * this->deltaTime;

    // ensure the angle stays in [0, 2*PI)
    while (this->currentAngle >= (2.0f * PI))
    {
        this->currentAngle -= (2.0f * PI);
    }
    while (this->currentAngle < 0.0f)
    {
        this->currentAngle += (2.0f * PI);
    }

    return this->currentAngle;
}


//finds the speed of the motor based on many factors
//returns values between -1 and 1
void kinimatics::findMotorSpeeds
        (
    float motorSpeeds[2], float currentRads, float translationSpeed,
        float translationRads, float spinSpeed 
        )
{
    // Treat `spinSpeed` as a -1..1 contribution and `translationSpeed` as 0..1
    // Combine translation (sinusoidal steer) with spin baseline, then clamp to -1..1.
    float spin = constrain( spinSpeed, -1.0f, 1.0f );
    float trans = constrain( translationSpeed, 0.0f, 1.0f );

    float s = sin( currentRads - translationRads ) * spin ;

    // left and right motor contributions
    motorSpeeds[0] = s * trans + spin; // left
    motorSpeeds[1] = s * trans - spin; // right

    motorSpeeds[0] = constrain( motorSpeeds[0], -1.0f, 1.0f );
    motorSpeeds[1] = constrain( motorSpeeds[1], -1.0f, 1.0f );
}


//calibrate the unknown radius based on the two acceleromiters
//this will only really work if the two are on the same side of the bot
void kinimatics::calibrateRadius( float acel1, float acel2 )
{
    //finds the magnatude of the radius based on the acceleration differences
    //used to calculate changing radius during movement
    float denom = ( acel1 - acel2 );
    if ( fabs( denom ) < 1e-6 ) return; // avoid divide-by-zero
    this -> foundRadius = ( acel1 * knownRadius ) / denom;
}


//allows tuning of the known radius
void kinimatics::increaseKnownRadius( float increaseBy )
{
    this -> knownRadius += increaseBy;
}


void kinimatics::decreaseKnownRadius( float decreaseBy )
{
    if( this -> knownRadius - decreaseBy > 0.00001 )
    {
        this -> knownRadius -= decreaseBy;
    }
    else
    {
        this -> knownRadius = 0.00001;
    }
}


//saves all of the tuned values to a file
bool kinimatics::saveValues()
{

    // Public API: persist the current tuned values to the flash filesystem.
    // Returns true on success, false otherwise.
    // First ensure a filesystem can be mounted (FFat or LittleFS), then
    // perform the actual write.
    if ( !openFile() )
    {
        return false;
    }

    bool ok = writeFile();
    return ok;

}

//writes the tuned values to the file
bool kinimatics::writeFile()
{
    const char * path = "/kinimatics.cfg";
    // Prefer the filesystem determined by openFile() (mountedFS)
    if ( this->mountedFS == 1 )
    {
        File f = FFat.open( path, "w" );
        if ( f )
        {
            f.print( "knownRadius=" );
            f.println( this->knownRadius, 6 );
            f.print( "foundRadius=" );
            f.println( this->foundRadius, 6 );
            f.close();
            return true;
        }
        // if write failed on FFat, try LittleFS as a fallback
        if ( LittleFS.begin() )
        {
            File f2 = LittleFS.open( path, "w" );
                if ( f2 )
                {
                    f2.print( "knownRadius=" );
                    f2.println( this->knownRadius, 6 );
                    f2.print( "foundRadius=" );
                    f2.println( this->foundRadius, 6 );
                    f2.close();
                    this->mountedFS = 2;
                    return true;
                }
        }
    }
    else if ( this->mountedFS == 2 )
    {
        File f = LittleFS.open( path, "w" );
        if ( f )
        {
            f.print( "knownRadius=" );
            f.println( this->knownRadius, 6 );
            f.print( "foundRadius=" );
            f.println( this->foundRadius, 6 );
            f.close();
            return true;
        }

        // fallback to FFat
        if ( FFat.begin() )
        {
            File f2 = FFat.open( path, "w" );
            if ( f2 )
            {
                f2.print( "knownRadius=" );
                f2.println( this->knownRadius, 6 );
                f2.print( "foundRadius=" );
                f2.println( this->foundRadius, 6 );
                f2.close();
                this->mountedFS = 1;
                return true;
            }
        }
    }
    return false;
}

// try to mount any available filesystem
bool kinimatics::openFile()
{
    // Attempt to initialize FFat first. Do NOT auto-format device here;
    // simply attempt to mount and return failure if unavailable.
    this->mountedFS = 0;

    bool ok = FFat.begin();
    if ( ok )
    {
        this->mountedFS = 1;
    }

    // fallback to LittleFS if FFat didn't mount
    if ( ! this->mountedFS )
    {
        ok = LittleFS.begin();
        if ( ok )
        {
            this->mountedFS = 2;
        }
    }

    // If neither FS mounted, try to enumerate partitions (same as OTA)
    if ( ! this->mountedFS )
    {
        esp_partition_iterator_t it = esp_partition_find( ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL );
        if ( it == NULL )
        {
        
        }
        else
        {
            do
            {
                const esp_partition_t* p = esp_partition_get( it );
                it = esp_partition_next( it );
            }
            while ( it );
            esp_partition_iterator_release( it );
        }
        // If no FS mounted, report failure (OTA hangs; we choose to return false)
        return false;
    }

    return true;
}

// Load tuned values from `/kinimatics.cfg` if available. The function
// checks FFat first, then LittleFS. The file is expected to contain
// `knownRadius=<float>` and `foundRadius=<float>` lines; parsed values
// override the in-memory values. Returns true if a file was found and
// parsed successfully, false otherwise.
bool kinimatics::loadValues()
{
    const char * path = "/kinimatics.cfg";

    // Ensure a filesystem is mounted and known
    if ( ! openFile() )
    {
        return false;
    }

    bool fileExists = false;
    if ( this->mountedFS == 1 ) fileExists = FFat.exists( path );
    else fileExists = LittleFS.exists( path );

    // If the mounted FS doesn't have the file, try the other FS
    if ( ! fileExists )
    {
        if ( this->mountedFS == 2 )
        {
            if ( FFat.begin() )
            {
                if ( FFat.exists( path ) )
                {
                    this->mountedFS = 1;
                    fileExists = true;
                }
            }
        }
        else
        {
            if ( LittleFS.begin() )
            {
                if ( LittleFS.exists( path ) )
                {
                    this->mountedFS = 2;
                    fileExists = true;
                }
            }
        }
    }

    if ( ! fileExists )
    {
        return false;
    }

    // Read file from whichever FS has it
    if ( this->mountedFS == 1 )
    {
        File f = FFat.open( path, "r" );
        if ( ! f )
        {
            return false;
        }
        while ( f.available() )
        {
            String line = f.readStringUntil('\n');
            line.trim();
            if ( line.startsWith( "knownRadius=" ) )
            {
                String v = line.substring( strlen("knownRadius=") );
                this->knownRadius = v.toFloat();
            }
            else if ( line.startsWith( "foundRadius=" ) || line.startsWith( "foudRadius=" ) )
            {
                // accept both the corrected and previous misspelled key
                size_t off = line.startsWith("foundRadius=") ? strlen("foundRadius=") : strlen("foudRadius=");
                String v = line.substring( off );
                this->foundRadius = v.toFloat();
            }
        }
        f.close();
        return true;
    }
    else if ( this->mountedFS == 2 )
    {
        File f = LittleFS.open( path, "r" );
        if ( ! f )
        {
            return false;
        }
        while ( f.available() )
        {
            String line = f.readStringUntil('\n');
            line.trim();
            if ( line.startsWith( "knownRadius=" ) )
            {
                String v = line.substring( strlen("knownRadius=") );
                this->knownRadius = v.toFloat();
            }
            else if ( line.startsWith( "foundRadius=" ) || line.startsWith( "foudRadius=" ) )
            {
                size_t off = line.startsWith("foundRadius=") ? strlen("foundRadius=") : strlen("foudRadius=");
                String v = line.substring( off );
                this->foundRadius = v.toFloat();
            }
        }
        f.close();
        return true;
    }
    return false;
}


//prints the kinimatics values to the screen
//allows for tuning on the robot and applying to the config file
void kinimatics::printValues()
{
    Serial.print( "Given Radius: " );
    Serial.println( this -> knownRadius, 10 );
    Serial.print( "Found Radius: " );
    Serial.println( this -> foundRadius, 10 );
}

