#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include<tuple>

unsigned long t_last;
float dt;

float gyro_multiplier;
float accel_multiplier;
Vector gyro_offset;

extern Quaternion rot_offset;

extern void SetupIMU();
extern int16_t* ReadSensor();
extern tuple<Vector, Quaternion> RawCorrection();
extern void SetupCalibration(data[6]);
extern void SleepIMU();

#endif
