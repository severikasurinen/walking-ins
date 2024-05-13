#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include<array>
using namespace std;

#include "quaternion_math.h"

extern float gyro_multiplier;
extern float accel_multiplier;
extern Vector gyro_offset;

extern void SetupIMU();
std::array<int16_t, 6> ReadSensor();
std::array<float, 6> RawCorrection();
extern void SetupCalibration();
void UpdateIMU();

#endif
