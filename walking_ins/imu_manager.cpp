#include "imu_manager.h"

#define IMU_ADDRESS 0x68      // MPU6050 I2C address
#define ACCELERATION_RANGE 1  // 0: 2g, 1: 4g, 2: 8g, 3: 16g
#define GYRO_RANGE 1          // 0: 250, 1: 500, 2: 1000, 3: 2000
#define LOW_PASS_FILTER 0     // 0: disabled, 1-6: increased filtering

void SetupIMU() {
  Wire.begin();           // Initialize comunication
  Wire.setClock(400000);  // 400kHz I2C clock. Comment this line if having compilation difficulties

  Wire.beginTransmission(IMU_ADDRESS);  // Start communication with MPU6050
  Wire.write(0x6B);                     // Talk to the register 6B
  Wire.write(0x01);                     // Reset device
  Wire.endTransmission(true);           //end the transmission

  // Configure Digital Low Pass Filter - default disabled
  Wire.beginTransmission(IMU_ADDRESS);
  Wire.write(0x1A);             // Talk to the DLPF_CFG register (1A hex)
  Wire.write(LOW_PASS_FILTER);  // Set the register bits
  Wire.endTransmission(true);

  // Configure Gyro Sensitivity - default +/- 250deg/s
  Wire.beginTransmission(IMU_ADDRESS);
  Wire.write(0x1B);             // Talk to the GYRO_CONFIG register (1B hex)
  Wire.write(GYRO_RANGE << 3);  // Set the register bits
  Wire.endTransmission(true);

  // Configure Accelerometer Sensitivity - default +/- 2g
  Wire.beginTransmission(IMU_ADDRESS);
  Wire.write(0x1C);                     //Talk to the ACCEL_CONFIG register (1C hex)
  Wire.write(ACCELERATION_RANGE << 3);  //Set the register bits
  Wire.endTransmission(true);

  Serial.println("MPU6050 setup done.");
}

void UpdateIMU() {
  Wire.beginTransmission(IMU_ADDRESS);
  Wire.write(0x3B);  // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(IMU_ADDRESS, 14, true);  // Read 14 registers total, each axis value is stored in 2 registers

  int16_t raw_data[7];

  raw_data[0] = Wire.read() << 8 | Wire.read();  // X-axis acceleration data
  raw_data[1] = Wire.read() << 8 | Wire.read();  // Y-axis acceleration data
  raw_data[2] = Wire.read() << 8 | Wire.read();  // Z-axis acceleration data

  raw_data[6] = Wire.read() << 8 | Wire.read();  // Temperature value in Celsius

  raw_data[3] = Wire.read() << 8 | Wire.read();  // X-axis gyro data
  raw_data[4] = Wire.read() << 8 | Wire.read();  // Y-axis gyro data
  raw_data[5] = Wire.read() << 8 | Wire.read();  // Z-axis gyro data

  Serial.print("x:");
  Serial.print(raw_data[0]);
  Serial.print(",");
  Serial.print("y:");
  Serial.print(raw_data[1]);
  Serial.print(",");
  Serial.print("z:");
  Serial.print(raw_data[2]);
  Serial.print(",");
  Serial.print("a:");
  Serial.print(raw_data[3]);
  Serial.print(",");
  Serial.print("b:");
  Serial.print(raw_data[4]);
  Serial.print(",");
  Serial.print("c:");
  Serial.println(raw_data[5]);
  /*
  for(int i = 0; i < 7; i++) {
    Serial.print(raw_data[i]);
    if(i < 6) {
      Serial.print(" | ");
    }
    else {
      Serial.println();
    }
  }
  */
}

/// Puts IMU into sleep mode.
void SleepIMU() {
  Wire.beginTransmission(IMU_ADDRESS);  // Start communication with MPU6050
  Wire.write(0x6B);                     // Talk to the register 6B
  Wire.write(0x40);                     // Set sleep bit to 1
  Wire.endTransmission(true);           //end the transmission
}
