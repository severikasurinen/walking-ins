#include "imu_manager.h"

#define IMU_ADDRESS 0x68      // MPU6050 I2C address
#define ACCELERATION_RANGE 2  // 0: 2g, 1: 4g, 2: 8g, 3: 16g
#define GYRO_RANGE 2          // 0: 250, 1: 500, 2: 1000, 3: 2000
#define LOW_PASS_FILTER 0     // 0: disabled, 1-6: increased filtering

#define G_VALUE 9.825         // Helsinki

#define SETUP_ITERATION 300


float gyro_multiplier = 1.0 / (131/pow(2, GYRO_RANGE)) * 1.025; // 16bit to deg/s multiplier, from datasheet, last multiplier tested empirically
float accel_multiplier = 1.0;                         // 16bit to m/s^2 multiplier, calculated in setup calibration
Vector gyro_offset = Vector();                                 // float offset to deg/s values, calculated in setup calibration


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
}

std::array<int16_t, 6> ReadSensor() {
  std::array<int16_t, 6> out_data;

  Wire.beginTransmission(IMU_ADDRESS);
  Wire.write(0x3B);  // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(IMU_ADDRESS, 14, true);  // Read 14 registers total, each axis value is stored in 2 registers

  out_data[0] = Wire.read() << 8 | Wire.read();  // X-axis acceleration data
  out_data[1] = Wire.read() << 8 | Wire.read();  // Y-axis acceleration data
  out_data[2] = Wire.read() << 8 | Wire.read();  // Z-axis acceleration data

  Wire.read() << 8 | Wire.read();  // Temperature value in Celsius

  out_data[3] = Wire.read() << 8 | Wire.read();  // X-axis gyro data
  out_data[4] = Wire.read() << 8 | Wire.read();  // Y-axis gyro data
  out_data[5] = Wire.read() << 8 | Wire.read();  // Z-axis gyro data
  
  return out_data;
}

std::array<float, 6> RawCorrection() {
  std::array<int16_t, 6> in_data = ReadSensor();

  Vector corrected_accel = Vector(in_data[0], in_data[1], in_data[2]);

  // Calibrating the raw data using setup calibration values

  corrected_accel.x = corrected_accel.x * accel_multiplier;
  corrected_accel.y = corrected_accel.y * accel_multiplier;
  corrected_accel.z = corrected_accel.z * accel_multiplier;

  Vector corrected_gyro = Vector(in_data[3], in_data[4], in_data[5]);
  corrected_gyro.x = corrected_gyro.x * gyro_multiplier - gyro_offset.x;
  corrected_gyro.y = corrected_gyro.y * gyro_multiplier - gyro_offset.y;
  corrected_gyro.z = corrected_gyro.z * gyro_multiplier - gyro_offset.z;

  std::array<float, 6> out_data = {
    corrected_accel.x,
    corrected_accel.y,
    corrected_accel.z,
    corrected_gyro.x,
    corrected_gyro.y,
    corrected_gyro.z
  };

  return out_data;
}

void SetupCalibration() {
  Vector linear_acc = Vector();
  Vector angular_vel = Vector();
  
  for (int i = 0; i < SETUP_ITERATION; i++) {
    std::array<int16_t, 6> raw_data = ReadSensor();
    linear_acc = linear_acc + (Vector(raw_data[0], raw_data[1], raw_data[2]) / SETUP_ITERATION);
    angular_vel = angular_vel + (Vector(raw_data[3], raw_data[4], raw_data[5]) / SETUP_ITERATION);
    delay(5);
  }

  Vector normalized_g = Vector(0, 0, 1);
  
  accel_multiplier = G_VALUE / linear_acc.GetMagnitude();
  gyro_offset = angular_vel * gyro_multiplier; //averaged angular velocity values
}

void UpdateIMU() {
  // Use RawCorrection() and last sensor data to calculate new values
  std::array<float, 6> new_data = RawCorrection();

  Serial.print(millis());
  Serial.print(",");
  Serial.print(new_data[0]);
  Serial.print(",");
  Serial.print(new_data[1]);
  Serial.print(",");
  Serial.print(new_data[2]);
  Serial.print(",");
  Serial.print(new_data[3]);
  Serial.print(",");
  Serial.print(new_data[4]);
  Serial.print(",");
  Serial.println(new_data[5]);
}
