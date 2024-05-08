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
extern Quaternion rot_offset;

extern Vector stopped_position;

extern Vector acceleration;
extern Vector angular_velocity;

extern Vector velocity;
extern Vector position;
extern Quaternion orientation;

extern bool device_moving;
extern uint32_t t_stopped;
extern float dt;
extern unsigned long t_last;

extern uint8_t print_iters;

extern void SetupIMU();
std::array<int16_t, 6> ReadSensor();
std::array<float, 6> RawCorrection();
bool MomentarilyStationary(float tolerance, float g);
extern void SetupCalibration();
void PartialCalibration();
void UpdateIMU();
extern void SleepIMU();

#endif
