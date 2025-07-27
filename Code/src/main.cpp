#include <Arduino.h>
//used for pwm to control the 3 ESCs for the drive
#include <ESP32Servo.h>
//used to communicate with the acceleromiter to get the angle
#include <GetAngle.h>
//used to make the LED show where the front of the robot is at all times
#include <HeaderLED.h>
//used fo the kinematics functions
#include <math.h>
//used to communicate with the Xbox controller
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>
//pins that the ESCs are solderd to 
#define ESCPin1 10
#define ESCPin2 42
#define Header 48
#define StatusLED 11


//all Web interface stuff
//used to host the web server

#include <WebInterface.h>
#include <OTAUpdates.h>

//Change for the debug interface or OTA uploads
//WebInterface Web1;
OTAUpdates Web2;




//creates the objects from the Servo library
Servo ESC1;
Servo ESC2;

//the minimum and maximum frequencys that the ESCs will respond to
const int minPulseWidth = 1000; // Minimum pulse width in microseconds (1ms)
const int maxPulseWidth = 2000; // Maximum pulse width in microseconds (2ms)

//this is the array that will hold the values that will be passed between the drive and kinimatic functions
//values between 0 and 100 will be used
//I am aware that "LeftRight" have no meaning to a robot spinning at 2k+ RPM, but its the only name I could think of
int LeftRight[2] = {50, 50};
//the base spinning percentage of the robot while it is active
//must be between 50 and 100
int spinSpeed = 50;

//creating all the objects for the various librarys
GetAngle angle;
HeaderLED Head(Header);
XboxSeriesXControllerESP32_asukiaaa::Core xboxController;

// Sets the speed of the motor received from the int LeftRight array
// It will then scale these to the pulse widths that can be sent to the ESCs
void setMotorSpeed() 
{
  //takes the value passed (between 0 and 100) and maps it to the PWM value (0 to 180)
  int esc1Speed = map(LeftRight[0], 0, 100, minPulseWidth, maxPulseWidth);
  int esc2Speed = map(LeftRight[1], 0, 100, minPulseWidth, maxPulseWidth);
  
  // Ensure the values are within the acceptable range
  esc1Speed = constrain(esc1Speed, minPulseWidth, maxPulseWidth);
  esc2Speed = constrain(esc2Speed, minPulseWidth, maxPulseWidth);

  //writes the calculated speed to the pin as a PWM value
  ESC1.writeMicroseconds(esc1Speed);
  ESC2.writeMicroseconds(esc2Speed);
}

//used to determine the motor speeds that will be needed to be able to translate across the arena
//pass the current position of the robot and the speed and direction that you will want to go
//will set the array LeftRight to the speeds that each motor will need to go
//Link to the kinematics graph https://www.desmos.com/calculator/uxo47nulse
void kinematics(float curentRads, float translationSpeed, float translationRads)
{
  LeftRight[1] = map(spinSpeed + sin(curentRads-translationRads) * (100 - spinSpeed)*translationSpeed, 0, 100, 50, 100);
  LeftRight[0] = map(spinSpeed - sin(curentRads-translationRads) * (100 - spinSpeed)*translationSpeed, 0, 100, 0, 50);
}


void conrollerRumble()
{
	XboxSeriesXHIDReportBuilder_asukiaaa::ReportBase repo;
	xboxController.writeHIDReport(repo);
	delay(2000);
}

//code here will run once when the robot turns on
//used to setup the motors and controler
void setup() 
{
  //Web1.start();
  Web2.start();
  // Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);

  pinMode(Header, OUTPUT);

  //angle.start();
  
  Serial.begin(9600);

  //starts the speed controlers
  ESC1.attach(ESCPin1, minPulseWidth, maxPulseWidth);
  ESC2.attach(ESCPin2, minPulseWidth, maxPulseWidth);

  LeftRight[0] = 0;
  LeftRight[1] = 0;
  setMotorSpeed();
  delay(5000);
  LeftRight[0] = 50;
  LeftRight[1] = 50;
  setMotorSpeed();
  delay(5000);

  //starts the connection to the controller
  xboxController.begin();

  //turns on the LED when the robot is set-up
  Head.RobotStopped();
  
}

void loop() 
{
  //refreshes the input from the xbox
  xboxController.onLoop();
  float x = ((xboxController.xboxNotif.joyLVert- 32767.5) / 65525) * 2;
  float y = ((xboxController.xboxNotif.joyRVert - 32767.5) / 65525) * 2;
  ////Web1.Talk(x, y, angle.getSpeed(), xboxController.xboxNotif.btnLB, angle.knownRadius);
  Web2.loop();

  //checks if th3e controler is connected
  if(xboxController.isConnected())
  {
    //checks if the controler has been fully set up
    if(!xboxController.isWaitingForFirstNotification())
    {
      //spins up the robot if the left bumper is pressed
      if(xboxController.xboxNotif.btnLB)
      {
        x = ((xboxController.xboxNotif.joyRHori- 32767.5) / 65525) * 2;
        y = ((xboxController.xboxNotif.joyRVert - 32767.5) / 65525) * 2;
        float stickSpeed = sqrt(pow(x, 2) + pow (y, 2));
        float stickRotation = atan2(y, x);
        //gets the kinematics based on the recived input from the controller
        //kinematics(angle.GetRads(), stickSpeed, stickRotation);
        //sets the motor speed to the value gotten from the kinematics function
        setMotorSpeed();
        //makes the header LED blink at the right time
        //Head.checkLED(angle.GetRads());
      }
      //if the left bumper isnt pressed stop the robot
      else
      {
        //turns on the LED
        Head.RobotStopped();
        //takes the xbox stick inputs and turns them into values between 0 and 1
        float rightTank = ((float(xboxController.xboxNotif.joyRVert)) / float(65525));
        float leftTank= ((float(xboxController.xboxNotif.joyLVert)) / float(65525));
        //passes to the drive loop
        LeftRight[1] = (y*15)+50;
        LeftRight[0] = (x*15)+50;
        setMotorSpeed();

      }
      //allows the user to reset the gyro and calibrate
      if (xboxController.xboxNotif.btnXbox)
      {
        //angle.Calibrate();
        //angle.ResetGyro();
      }
      //adjust the radius for tuning
      /*if(xboxController.xboxNotif.btnDirDown && angle.knownRadius > 0)
      {
        angle.knownRadius -= 0.00001;
      }
      if(xboxController.xboxNotif.btnDirUp)
      {
        angle.knownRadius += 0.00001;
      }*/
    }
  }
  //blinks the LED and stops the motors if the controler is not connected
  else
  {
    Head.blink();
    //stops the motor
    LeftRight[1] = 50;
    LeftRight[0] = 50;
    setMotorSpeed();
  }
}
