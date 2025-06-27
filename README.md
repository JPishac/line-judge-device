# line-judge-device
Firmware and schematics for an ESP32-based Real-Time Throw Impact Detector.

## Hardware Connections
- **ESP32 GPIO17 (SDA)** → MPU-6050 SDA (both modules in parallel)
- **ESP32 GPIO16 (SCL)** → MPU-6050 SCL (both modules in parallel)
- **MPU-6050 VCC** → 3.3 V on ESP32  
- **MPU-6050 GND** → GND on ESP32  
- **ESP32 GPIO13 (PWM)** → SG90 servo signal  
- **SG90 VCC** → 5 V supply; **SG90 GND** → GND

## Usage
- Power the ESP32 via USB or a 5 V adapter.
- On boot, the servo wiggles to confirm calibration.
- The device reads each MPU-6050’s Z-axis acceleration at ~250 Hz, applies a high-pass filter, and watches for spikes above 2 m/s².
- When a left-module impact is detected, the servo moves to 45° (“left”); for a right-module impact, it moves to 135° (“right”).
- After 3 seconds it returns to the center (90°) and awaits the next event.
