#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include <Wire.h>
#include "Arduino.h"

#define IMU_ADDRESS 0x68      // MPU6050 I2C address
#define ACCELERATION_RANGE 1  // 0: 2g, 1: 4g, 2: 8g, 3: 16g
#define GYRO_RANGE 1          // 0: 250, 1: 500, 2: 1000, 3: 2000
#define LOW_PASS_FILTER 0     // 0: disabled, 1-6: increased filtering

extern void SetupIMU();
extern void UpdateIMU();

#endif
