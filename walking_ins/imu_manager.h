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

extern Vector acceleration;
extern Vector angular_velocity;

extern Vector velocity;
extern Vector position;
extern Quaternion orientation;

extern unsigned long t_last;

extern bool device_moving;
extern uint32_t t_stopped;
extern uint32_t t_moved;
extern Vector stopped_position;

extern uint32_t t_calib;
extern float calib_gyro_max;
extern float calib_gyro_min;
extern uint32_t t_last_calib;
extern float last_calib_min;
extern uint16_t avg_iterations;
extern Vector avg_acceleration;

extern uint8_t print_iters;

extern void SetupIMU();
std::array<int16_t, 6> ReadSensor();
std::array<float, 6> RawCorrection();
uint8_t MomentarilyStationary(float accel_norm, float gyro_norm);
extern void SetupCalibration();
void PartialCalibration(Vector avg_accel);
void SetMoving(bool new_moving);
void UpdateIMU();
extern void SleepIMU();

#endif
