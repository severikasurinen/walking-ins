#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include <Arduino.h>
#include <Wire.h>

extern void SetupIMU();
extern void UpdateIMU();
extern void SleepIMU();

#endif
