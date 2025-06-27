# line-judge-device
Firmware and schematics for an ESP32-based Real-Time Throw Impact Detector.

Hardware Connections:

ESP32 GPIO17 (SDA) → both MPU-6050 modules’ SDA pins
ESP32 GPIO16 (SCL) → both MPU-6050 modules’ SCL pins
MPU-6050 VCC → ESP32 3.3 V
MPU-6050 GND → ESP32 GND
ESP32 GPIO13 (PWM) → SG90 servo signal line
SG90 servo VCC → 5 V supply; SG90 servo GND → ESP32 GND

Operation:

Power the ESP32 via USB or a 5 V adapter.
On boot, the servo wiggles to confirm calibration.
The device reads each MPU-6050’s Z-axis acceleration at ~250 Hz, applies a high-pass filter, and watches for spikes above 2 m/s².
When a left-module impact is detected, the servo moves to 45° (“left”); for a right-module impact, it moves to 135° (“right”).
After 3 seconds it returns to the center (90°) and awaits the next event.
