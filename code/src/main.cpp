#include <Arduino.h>
#include <GetAngle.h>
#include <HeaderLED.h>
#include <math.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>
#include "WebInterface.h"
#include "OTAUpdates.h"
#include "kinimatics.h"

// ========== Pin assignments ==========
#define ESCPin1 10
#define ESCPin2 42
#define Header 48
#define StatusLED 11

// ========== ESC timing ==========
const int minPulseWidth = 1000; // µs
const int maxPulseWidth = 2000; // µs

int deltaTime = 0;
bool statusLEDState = true;
bool serverState = false;

// ========== Speed variables ==========
// Motor values now range from -1.0 to +1.0
float LeftRight[2] = {0.0, 0.0};
// Spin speed = baseline motor value (0–1 range)
float spinSpeed = 0.75;

GetAngle angle;
HeaderLED Head(Header);
XboxSeriesXControllerESP32_asukiaaa::Core xboxController;
OTAUpdates Web2;
kinimatics kini;

// Map motor speed [-1,1] → PWM duty for ESC
void setMotorSpeed();

void setup() {
  pinMode(Header, OUTPUT);
  pinMode(StatusLED, OUTPUT);

  angle.start();
  Serial.begin(9600);

  // Setup PWM (50 Hz, 12-bit resolution)
  const int pwmFreq = 50;
  const int pwmRes = 12;
  ledcSetup(0, pwmFreq, pwmRes);
  ledcAttachPin(ESCPin1, 0);

  ledcSetup(1, pwmFreq, pwmRes);
  ledcAttachPin(ESCPin2, 1);

  // Arm both motors at neutral
  ledcWrite(0, 350);
  ledcWrite(1, 350);
  delay(2500);
  digitalWrite(StatusLED, HIGH);

  xboxController.begin();
  Head.RobotStopped();
}

void loop() {
  xboxController.onLoop();
  float x = 0, y = 0;

  if (serverState) {
    Web2.loop();
  }
  angle.loop();

  if (xboxController.isConnected()) {
    digitalWrite(StatusLED, HIGH);

    if (!xboxController.isWaitingForFirstNotification()) {
      if (xboxController.xboxNotif.btnRB) {
        // Right stick as translation vector
        x = (xboxController.xboxNotif.joyRHori - 32767.5f) / 32767.5f;
        y = (xboxController.xboxNotif.joyRVert - 32767.5f) / 32767.5f;
        float stickSpeed = sqrtf(x * x + y * y);
        float stickRotation = atan2f(y, x);

        *LeftRight = *kini.getSpeed(angle.getCurrentRads(), stickSpeed, stickRotation, spinSpeed);
        setMotorSpeed();
        Head.checkLED(angle.getCurrentRads());
      } else if (xboxController.xboxNotif.btnLB) {
        // Simple spin mode
        LeftRight[0] = 0.5;
        LeftRight[1] = 0.5;
        setMotorSpeed();
      } else {
        // Tank drive
        Head.RobotStopped();
        float rightTank = (float)xboxController.xboxNotif.joyRVert / 32767.5f;
        float leftTank  = (float)xboxController.xboxNotif.joyLVert / 32767.5f;

        LeftRight[0] = constrain(leftTank, -1.0, 1.0);
        LeftRight[1] = constrain(rightTank, -1.0, 1.0);
        setMotorSpeed();
      }

      if (xboxController.xboxNotif.btnXbox) {
        LeftRight[0] = 0;
        LeftRight[1] = 0;
        setMotorSpeed();
        Web2.start();
        serverState = true;
      }

      if (xboxController.xboxNotif.btnShare) {
        angle.Calibrate();
        angle.ResetGyro();
      }

      if (xboxController.xboxNotif.btnDirDown && angle.knownRadius > 0) {
        angle.knownRadius -= 0.00001;
      }
      if (xboxController.xboxNotif.btnDirUp) {
        angle.knownRadius += 0.00001;
      }
    }
  } else {
    // Controller disconnected
    deltaTime += millis();
    if (deltaTime >= 1000) {
      statusLEDState = !statusLEDState;
      digitalWrite(StatusLED, statusLEDState ? HIGH : LOW);
      deltaTime = 0;
    }

    // Stop motors
    LeftRight[0] = 0;
    LeftRight[1] = 0;
    setMotorSpeed();
  }
}


//Sets motor drive speed
void setMotorSpeed() {
  // Map -1..1 → 1000..2000 µs → duty
  int esc1Micros = map((int)((LeftRight[0] + 1.0) * 1000), 0, 2000, 205, 410);
  int esc2Micros = map((int)((LeftRight[1] + 1.0) * 1000), 0, 2000, 205, 410);

  ledcWrite(0, esc1Micros);
  ledcWrite(1, esc2Micros);
}
