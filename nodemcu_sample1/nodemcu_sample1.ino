#include "analogWrite.h"
#include <ESP32Servo.h>

#define PIN1 32
#define PIN2 33
#define PIN3 25
#define PIN4 26

#define PIN_WAIST 13
#define PIN_LEFT_ARM 23

Servo waist;
Servo leftgArm;

void setup() {

  waist.setPeriodHertz(50);    // standard 50 hz servo
  waist.attach(PIN_WAIST, 1000, 2000);

  leftgArm.setPeriodHertz(50);    // standard 50 hz servo
  leftgArm.attach(PIN_LEFT_ARM, 1000, 2000);


  pinMode(PIN1, OUTPUT);
  pinMode(PIN2, OUTPUT);
  pinMode(PIN3, OUTPUT);
  pinMode(PIN4, OUTPUT);

  waist.write(90);
  leftgArm.write(90);

  delay(1000 * 10);
}

bool forward = true;
int forwardIndex = 0;

int direction = 0;
int leftArmRadius = 0;

void loop() {
  if (forward) {
    analogWrite(PIN1, 255);
    analogWrite(PIN2, 0);
    analogWrite(PIN3, 255);
    analogWrite(PIN4, 0);
  } else {
    analogWrite(PIN1, 0);
    analogWrite(PIN2, 255);
    analogWrite(PIN3, 0);
    analogWrite(PIN4, 255);
  }

  delay(1000);

  forwardIndex = (forwardIndex + 1) % 5;
  if (forwardIndex == 0) {
    forward = !forward;
  }

  direction = (direction + 10) % 180 + 90;
  waist.write(direction);
  delay(500);

  leftArmRadius = (leftArmRadius + 15) % 45 + 90;
  leftgArm.write(leftArmRadius);
  delay(500);
}
