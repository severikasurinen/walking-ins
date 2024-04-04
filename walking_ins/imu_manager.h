#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include <Arduino.h>
#include <Wire.h>

#include "quaternion_math.h"

float gyro_multiplier;
float accel_multiplier;
Vector gyro_offset;
Quaternion rot_offset;

extern Vector velocity;
extern Vector position;
extern Quaternion rotation;

extern bool device_moving;
float dt;
uint32_t t_stopped = 0;
unsigned long t_last;

extern void SetupIMU();
int16_t* ReadSensor();
float* RawCorrection();
bool MomentarilyStationary(float tolerance, float g);
extern void SetupCalibration();
void PartialCalibration();
void UpdateIMU();
extern void SleepIMU();

#endif
