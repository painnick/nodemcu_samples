#include "analogWrite.h"
#include <ESP32Servo.h>
#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"    // disable brownout problems

#define PIN_TRACK_A1 32
#define PIN_TRACK_A2 33
#define PIN_TRACK_B1 25
#define PIN_TRACK_B2 26

#define PIN_WAIST 21
#define PIN_LEFT_ARM 19
#define PIN_RIGHT_ARM 18

#define PINT_LEFT_GUN 5
#define PINT_RIGHT_GUN 17

Servo waist;
Servo leftArm, rightArm;

void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);
  // Serial.setDebugOutput(false);

  waist.setPeriodHertz(50);    // standard 50 hz servo
  waist.attach(PIN_WAIST, 1000, 2000);
  waist.write(0);
  delay(500);

  leftArm.setPeriodHertz(50);    // standard 50 hz servo
  leftArm.attach(PIN_LEFT_ARM, 1000, 2000);
  leftArm.write(0);
  delay(500);

  rightArm.setPeriodHertz(50);    // standard 50 hz servo
  rightArm.attach(PIN_LEFT_ARM, 1000, 2000);
  rightArm.write(0);
  delay(500);

  pinMode(PIN_TRACK_A1, OUTPUT);
  pinMode(PIN_TRACK_A2, OUTPUT);
  pinMode(PIN_TRACK_B1, OUTPUT);
  pinMode(PIN_TRACK_B2, OUTPUT);

  pinMode(PINT_LEFT_GUN, OUTPUT);
  pinMode(PINT_RIGHT_GUN, OUTPUT);

  Serial.println("Setup.(1/5)");
  delay(1000);
  Serial.println("Setup..(2/5)");
  delay(1000);
  Serial.println("Setup...(3/5)");
  delay(1000);
  Serial.println("Setup....(4/5)");
  delay(1000);
  Serial.println("Setup.....(5/5)");
  delay(1000);
}

bool forward = true;
int forwardIndex = 0;

int direction = 0;
int leftArmRadius = 0;
int rightArmRadius = 0;

void loop() {

  Serial.println("===== Loop start =====");

  if (forward) {
    digitalWrite(PIN_TRACK_A1, HIGH);
    digitalWrite(PIN_TRACK_A2, LOW);
    digitalWrite(PIN_TRACK_B1, HIGH);
    digitalWrite(PIN_TRACK_B2, LOW);
    Serial.println("Track set Forward");
  } else {
    digitalWrite(PIN_TRACK_A1, LOW);
    digitalWrite(PIN_TRACK_A2, HIGH);
    digitalWrite(PIN_TRACK_B1, LOW);
    digitalWrite(PIN_TRACK_B2, HIGH);
    Serial.println("Track set Backward");
  }

  delay(1000);

  forwardIndex = (forwardIndex + 1) % 5;
  if (forwardIndex == 0) {
    forward = !forward;
  }

  direction = (direction + 10) % 180;
  waist.write(direction);
  Serial.print("Waist set ");
  Serial.println(direction);

  delay(500);

  leftArmRadius = (leftArmRadius + 15) % 45;
  leftArm.write(leftArmRadius);
  Serial.print("Left arm set");
  Serial.println(leftArmRadius);

  delay(500);

  if (leftArmRadius < 15) {
    analogWrite(PINT_LEFT_GUN, 255);
    Serial.println("Left gun set On");
  } else {
    analogWrite(PINT_LEFT_GUN, 0);
    Serial.println("Left gun set Off");
  }

  rightArmRadius = (rightArmRadius + 10) % 60;
  rightArm.write(rightArmRadius);
  Serial.print("Right arm set ");
  Serial.println(rightArmRadius);

  delay(500);

  if (rightArmRadius < 10) {
    analogWrite(PINT_RIGHT_GUN, 255);
    Serial.println("Right gun set On");
  } else {
    analogWrite(PINT_RIGHT_GUN, 0);
    Serial.println("Right gun set Off");
  }
}
