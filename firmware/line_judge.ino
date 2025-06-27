#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP32Servo.h>

const int SDA_PIN = 17;    // ESP32 I²C SDA
const int SCL_PIN = 16;    // ESP32 I²C SCL
const int SERVO_PIN = 13;  // ESP32 PWM pin

Adafruit_MPU6050 mpuL;  // Left MPU-6050 (0x68)
Adafruit_MPU6050 mpuR;  // Right MPU-6050 (0x69)

const int CALIB_SAMPLES = 250;  // ~0.75 s per MPU @ ~300 Hz

float baselineL = 0;
float baselineR = 0;

// High-pass filter parameters
const float HPF_CUTOFF = 30.0f;       // remove <20 Hz
const float SAMPLE_RATE_HZ = 300.0f;  // loop ~300 Hz
const float DT = 1.0f / SAMPLE_RATE_HZ;
const float TAU = 1.0f / (2.0f * PI * HPF_CUTOFF);
const float ALPHA = TAU / (TAU + DT);
const float HPF_THRESHOLD = 2.0f;  // m/s² — tune this

// HPF state
float prevZL = 0, hpfL = 0;
float prevZR = 0, hpfR = 0;

const int LEFT_HIT_ANGLE = 45;
const int CENTER_ANGLE = 90;
const int RIGHT_HIT_ANGLE = 135;
const int ERR_LEFT_ANGLE = 0;
const int ERR_RIGHT_ANGLE = 180;

Servo flagServo;
bool firstBounceDetected = false;
unsigned long bounceTimeMs = 0;

void setup() {
// 1) Servo wiggle to signal calibration
flagServo.attach(SERVO_PIN);
flagServo.write(CENTER_ANGLE);
delay(200);
flagServo.write(60);
delay(300);
flagServo.write(120);
delay(300);
flagServo.write(CENTER_ANGLE);
delay(200);

// 2) I²C @400kHz + MPU init
Wire.begin(SDA_PIN, SCL_PIN, 400000);
if (!mpuL.begin(0x68)) {
 flagServo.write(ERR_LEFT_ANGLE);
 while (1) delay(10);
}
if (!mpuR.begin(0x69)) {
 flagServo.write(ERR_RIGHT_ANGLE);
 while (1) delay(10);
}
delay(100);

// 3) ±4 g, 94 Hz DLPF
mpuL.setAccelerometerRange(MPU6050_RANGE_4_G);
mpuL.setFilterBandwidth(MPU6050_BAND_94_HZ);
mpuR.setAccelerometerRange(MPU6050_RANGE_4_G);
mpuR.setFilterBandwidth(MPU6050_BAND_94_HZ);

// 4) Calibrate LEFT baseline
{
 double sumZ = 0.0;
 for (int i = 0; i < CALIB_SAMPLES; i++) {
   sensors_event_t e, g, t;
   mpuL.getEvent(&e, &g, &t);
   sumZ += e.acceleration.z;
   delay(3);
 }
 baselineL = sumZ / CALIB_SAMPLES;
 prevZL = baselineL;  // init HPF memory
 hpfL = 0;
}

// 5) Calibrate RIGHT baseline
{
 double sumZ = 0.0;
 for (int i = 0; i < CALIB_SAMPLES; i++) {
   sensors_event_t e, g, t;
   mpuR.getEvent(&e, &g, &t);
   sumZ += e.acceleration.z;
   delay(3);
 }
 baselineR = sumZ / CALIB_SAMPLES;
 prevZR = baselineR;
 hpfR = 0;
}

// 6) Final center
flagServo.write(CENTER_ANGLE);
delay(200);
}

void loop() {
// Read LEFT Z
sensors_event_t eL, gL, tL;
mpuL.getEvent(&eL, &gL, &tL);
float zL = eL.acceleration.z;

// HPF LEFT
hpfL = ALPHA * (hpfL + (zL - prevZL));
prevZL = zL;

// Read RIGHT Z
sensors_event_t eR, gR, tR;
mpuR.getEvent(&eR, &gR, &tR);
float zR = eR.acceleration.z;

// HPF RIGHT
hpfR = ALPHA * (hpfR + (zR - prevZR));
prevZR = zR;

// First-bounce: pick whichever HPF spike is larger
if (!firstBounceDetected) {
 bool hitL = fabsf(hpfL) > HPF_THRESHOLD;
 bool hitR = fabsf(hpfR) > HPF_THRESHOLD;

 if (hitL || hitR) {
   if (fabsf(hpfL) > fabsf(hpfR)) {
     flagServo.write(LEFT_HIT_ANGLE);
   } else {
     flagServo.write(RIGHT_HIT_ANGLE);
   }
   firstBounceDetected = true;
   bounceTimeMs = millis();
 }
}
// Hold then recenter
else if (millis() - bounceTimeMs >= 3000UL) {
 flagServo.write(CENTER_ANGLE);
 firstBounceDetected = false;
}

// No delay() → ~200–300 Hz loop
}









