#include "analogWrite.h"
#include <ESP32Servo.h>
#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"    // disable brownout problems

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define PIN_TRACK_A1 32
#define PIN_TRACK_A2 33
#define PIN_TRACK_B1 25
#define PIN_TRACK_B2 26

#define PIN_BODY 21
#define PIN_LEFT_ARM 19
#define PIN_RIGHT_ARM 18

#define PIN_LEFT_GUN 5
#define PIN_RIGHT_GUN 17

#define SG90_DELAY 500 // ms
#define MINI_SG_DELAY 500 // ms

#define ANGLE_CENTER 90

#define BODY_ANGLE_MIN 30
#define BODY_ANGLE_MAX 150

#define ARM_ANGLE_MIN 45
#define ARM_ANGLE_MAX 135

typedef enum {
  UNKNOWN_SIDE = -1,
  LEFT,
  RIGHT,
  BOTH
} GUNTANK_SIDE;

typedef enum {
  UNKNOWN_DIRECTION = -1,
  FORWARD,
  BACKWARD,
  CENTER
} GUNTANK_DIRECTION;

Servo body;
Servo arms[2];
int gunPins[2] = {PIN_LEFT_GUN, PIN_RIGHT_GUN};
int trackPins[4] = {PIN_TRACK_A1, PIN_TRACK_A2, PIN_TRACK_B1, PIN_TRACK_B2};

BluetoothSerial SerialBT;

void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);
  // Serial.setDebugOutput(false);

  SerialBT.begin("Guntank"); //Bluetooth device name

  pinMode(PIN_TRACK_A1, OUTPUT);
  pinMode(PIN_TRACK_A2, OUTPUT);
  pinMode(PIN_TRACK_B1, OUTPUT);
  pinMode(PIN_TRACK_B2, OUTPUT);

  pinMode(PIN_LEFT_GUN, OUTPUT);
  pinMode(PIN_RIGHT_GUN, OUTPUT);

  body.setPeriodHertz(50);    // standard 50 hz servo
  body.attach(PIN_BODY, 1000, 2000);
  turnBody(GUNTANK_DIRECTION::CENTER, 0);

  arms[GUNTANK_SIDE::LEFT].setPeriodHertz(50);    // standard 50 hz servo
  arms[GUNTANK_SIDE::LEFT].attach(PIN_LEFT_ARM, 1000, 2000);
  turnArm(GUNTANK_SIDE::LEFT, GUNTANK_DIRECTION::CENTER, 0);

  arms[GUNTANK_SIDE::RIGHT].setPeriodHertz(50);    // standard 50 hz servo
  arms[GUNTANK_SIDE::RIGHT].attach(PIN_RIGHT_ARM, 1000, 2000);
  turnArm(GUNTANK_SIDE::RIGHT, GUNTANK_DIRECTION::CENTER, 0);

  delay(1000);
}

char command[16];

void loop() {

  int index = 0;
  while (SerialBT.available()) {
    int c = SerialBT.read();
    command[index] = c;
    index++;
  }
  if (index == 0) {
    delay(200);
    return;
  }
  command[index] = 0;
  Serial.print("Command : [");
  Serial.print(command);
  Serial.println("]");

  char side;
  char direction;
  int angle;
  switch (command[0])
  {
  case 'B': // Body. B (F/B/C) 15
    sscanf(command, "B %c %d", &direction, &angle);
    turnBody(convertToDirection(direction), angle);
    break;
  case 'A': // Arm. A (L/R/B) (F/B/C) 15
    sscanf(command, "A %c %c %d", &side, &direction, &angle);
    turnArm(convertToSide(side), convertToDirection(direction), angle);
    break;
  case 'F': // Fire. F (L/R/B)
    sscanf(command, "F %c", &side);
    fire(convertToSide(side));
    break;
  case 'T': // Track. T (L/R/B) (F/B/C)
    sscanf(command, "T %c %c", &side, &direction);
    track(convertToSide(side), convertToDirection(direction));
    break;
  }
}

GUNTANK_SIDE convertToSide(char c) {
  if (c == 'L')
    return GUNTANK_SIDE::LEFT;
  else if (c == 'R')
    return GUNTANK_SIDE::RIGHT;
  else if (c == 'B')
    return GUNTANK_SIDE::BOTH;
  else
    return GUNTANK_SIDE::UNKNOWN_SIDE;
}

GUNTANK_DIRECTION convertToDirection(char c) {
  if (c == 'F')
    return GUNTANK_DIRECTION::FORWARD;
  else if (c == 'B')
    return GUNTANK_DIRECTION::BACKWARD;
  else if (c == 'C')
    return GUNTANK_DIRECTION::CENTER;
  else
    return GUNTANK_DIRECTION::UNKNOWN_DIRECTION;
}

int bodyAngle = ANGLE_CENTER;
void turnBody(GUNTANK_DIRECTION direction, int angle) {
  Serial.print("turnBody dir:");
  Serial.print(direction);
  Serial.print(" angle:");
  Serial.println(angle);

  if (direction == GUNTANK_DIRECTION::UNKNOWN_DIRECTION)
    return;
  int newAngle = ANGLE_CENTER;
  if (direction != GUNTANK_DIRECTION::CENTER) {
    newAngle = bodyAngle + (direction == GUNTANK_DIRECTION::FORWARD ? angle : -angle);
    newAngle = min(BODY_ANGLE_MAX, newAngle);
    newAngle = max(BODY_ANGLE_MIN, newAngle);
  }
  bodyAngle = newAngle;
  Serial.printf("Body Angle %d\n", bodyAngle);
  body.write(bodyAngle);
  delay(SG90_DELAY);
}

int armAngles[2] = {ANGLE_CENTER, ANGLE_CENTER};
void turnArm(GUNTANK_SIDE side, GUNTANK_DIRECTION direction, int angle) {
  Serial.print("turnArm side:");
  Serial.print(side);
  Serial.print(" dir:");
  Serial.print(direction);
  Serial.print(" angle:");
  Serial.println(angle);

  if (side == GUNTANK_SIDE::UNKNOWN_SIDE)
    return;
  if (direction == GUNTANK_DIRECTION::UNKNOWN_DIRECTION)
    return;
  int newAngle = ANGLE_CENTER;
  if (direction != GUNTANK_DIRECTION::CENTER) {
    newAngle = armAngles[side] + (direction == GUNTANK_DIRECTION::FORWARD ? angle : -angle);
    newAngle = min(ARM_ANGLE_MAX, newAngle);
    newAngle = max(ARM_ANGLE_MIN, newAngle);
  }
  armAngles[side] = newAngle;
  arms[side].write(armAngles[side]);
  delay(MINI_SG_DELAY);
}

void fire(GUNTANK_SIDE side) {
  Serial.print("fire side:");
  Serial.println(side);

  if (side == GUNTANK_SIDE::UNKNOWN_SIDE)
    return;
  bool isOn = true;
  for(int i = 0; i < 6; i ++) {
    if (side == GUNTANK_SIDE::BOTH) {
      digitalWrite(gunPins[GUNTANK_SIDE::LEFT], isOn ? HIGH : LOW);
      digitalWrite(gunPins[GUNTANK_SIDE::RIGHT], isOn ? HIGH : LOW);
      delay(100);
    } else {
      digitalWrite(gunPins[side], isOn ? HIGH : LOW);
      delay(100);
    }
    isOn = !isOn;
  }
}

void track(GUNTANK_SIDE side, GUNTANK_DIRECTION direction) {
  Serial.print("track side:");
  Serial.print(side);
  Serial.print(" dir:");
  Serial.println(direction);

  if (side == GUNTANK_SIDE::UNKNOWN_SIDE)
    return;
  if (direction == GUNTANK_DIRECTION::UNKNOWN_DIRECTION)
    return;
  switch (side) {
    case GUNTANK_SIDE::LEFT:
      switch (direction) {
      case GUNTANK_DIRECTION::FORWARD:
        digitalWrite(trackPins[0], HIGH);
        digitalWrite(trackPins[1], LOW);
        break;
      case GUNTANK_DIRECTION::BACKWARD:
        digitalWrite(trackPins[0], LOW);
        digitalWrite(trackPins[1], HIGH);
        break;
      case GUNTANK_DIRECTION::CENTER:
        digitalWrite(trackPins[0], LOW);
        digitalWrite(trackPins[1], LOW);
        break;
      }
    break;
    case GUNTANK_SIDE::RIGHT:
      switch (direction) {
      case GUNTANK_DIRECTION::FORWARD:
        digitalWrite(trackPins[2], HIGH);
        digitalWrite(trackPins[3], LOW);
        break;
      case GUNTANK_DIRECTION::BACKWARD:
        digitalWrite(trackPins[2], LOW);
        digitalWrite(trackPins[3], HIGH);
        break;
      case GUNTANK_DIRECTION::CENTER:
        digitalWrite(trackPins[2], LOW);
        digitalWrite(trackPins[3], LOW);
        break;
      }
    break;
    case GUNTANK_SIDE::BOTH:
      switch (direction) {
      case GUNTANK_DIRECTION::FORWARD:
        digitalWrite(trackPins[0], HIGH);
        digitalWrite(trackPins[1], LOW);
        digitalWrite(trackPins[2], HIGH);
        digitalWrite(trackPins[3], LOW);
        break;
      case GUNTANK_DIRECTION::BACKWARD:
        digitalWrite(trackPins[0], LOW);
        digitalWrite(trackPins[1], HIGH);
        digitalWrite(trackPins[2], LOW);
        digitalWrite(trackPins[3], HIGH);
        break;
      case GUNTANK_DIRECTION::CENTER:
        digitalWrite(trackPins[0], LOW);
        digitalWrite(trackPins[1], LOW);
        digitalWrite(trackPins[2], LOW);
        digitalWrite(trackPins[3], LOW);
        break;
      }
    break;
  } 
}
