#include "imu_manager.h"

#define DEBUG_MODE true

#define IMU_ADDRESS 0x68      // MPU6050 I2C address
#define ACCELERATION_RANGE 1  // 0: 2g, 1: 4g, 2: 8g, 3: 16g
#define GYRO_RANGE 1          // 0: 250, 1: 500, 2: 1000, 3: 2000
#define LOW_PASS_FILTER 0     // 0: disabled, 1-6: increased filtering

#define G_VALUE 9.825         // Helsinki

#define SETUP_ITERATION 200
#define MOVING_TOLERANCE 0.1  // in m/s^2
#define STOP_TIME 200         // Time in ms required to stay still for setting device_moving to false

float gyro_multiplier = 1.0 / (131/pow(2, GYRO_RANGE)); // 16bit to deg/s multiplier, from datasheet
float accel_multiplier = 1.0;                         // 16bit to m/s^2 multiplier, calculated in setup calibration
Vector gyro_offset = Vector();                                 // float offset to deg/s values, calculated in setup calibration
Quaternion rot_offset = Quaternion();                              // Rotation from local to global vectors, calculated in setup calibration

Vector last_corrected_acceleration = Vector();
Vector last_corrected_velocity = Vector();
Vector last_corrected_position = Vector();
Quaternion last_corrected_rotation = Quaternion();

Vector acceleration = Vector();
Vector velocity = Vector();
Vector position = Vector();
Quaternion rotation = Quaternion();

bool device_moving = false;
uint32_t t_stopped = 0;     // Since when device has been still?
float dt = 0;
unsigned long t_last = 0;

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

  if (micros() < t_last) {
    dt = (ULONG_MAX - t_last + micros()) / 1000000.0;
  }
  else {
    dt = (micros() - t_last) / 1000000.0;
  }
        
  t_last = micros();
  
  return out_data;
}

std::array<float, 7> RawCorrection() {
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

  Quaternion corrected_rot = Quaternion(EulerToQuaternion(corrected_gyro.x, corrected_gyro.y, corrected_gyro.z));

  std::array<float, 7> out_data = {
    corrected_accel.x,
    corrected_accel.y,
    corrected_accel.z,
    corrected_rot.w,
    corrected_rot.x,
    corrected_rot.y,
    corrected_rot.z
  };

  return out_data;
}


bool MomentarilyStationary(Vector in_accel) { //returns true if the norm of the linear acceleration is g within tolerance
  float norm = sqrt(VectorDot(in_accel, in_accel));
  if((norm + MOVING_TOLERANCE < G_VALUE) || (norm - MOVING_TOLERANCE > G_VALUE)) {
    return true;
  } else {
    return false;
  }
}


void SetupCalibration() {
  std::array<int16_t, 6> init_data = ReadSensor();
  Vector linear_acc = Vector(init_data[0],init_data[1],init_data[2]);
  Vector angular_vel = Vector(init_data[3], init_data[4], init_data[5]);


  for (int i = 2; i <= SETUP_ITERATION; i++) {
    std::array<int16_t, 6> raw_data = ReadSensor();
    Vector acc_correction = Vector(raw_data[0], raw_data[1], raw_data[2]);
    Vector ang_correction = Vector(raw_data[3], raw_data[4], raw_data[5]);
    linear_acc = linear_acc + acc_correction;
    angular_vel = angular_vel + ang_correction;
  }


  Vector normalized_g = Vector(0, 0, 1);
  

  gyro_offset = angular_vel / SETUP_ITERATION; //averaged angular velocity values
  rot_offset = OffsetQ(linear_acc / SETUP_ITERATION, normalized_g);
}

void PartialCalibration() {
  std::array<int16_t, 6> raw_data = ReadSensor();
  // Set velocity to 0, reset values to first update of no movement
  velocity = Vector();
}

void UpdateIMU() {
  // Use RawCorrection() and last sensor data to calculate new values

  std::array<float, 7> new_data = RawCorrection();
  Vector new_accel = Vector(new_data[0], new_data[1], new_data[2]);
  Quaternion new_rot = Quaternion(new_data[3], new_data[4], new_data[5], new_data[6]);

  if(DEBUG_MODE) {
    Serial.print("x:");
    Serial.print(new_accel.x);
    Serial.print(",");
    Serial.print("y:");
    Serial.print(new_accel.y);
    Serial.print(",");
    Serial.print("z:");
    Serial.print(new_accel.z);
    Serial.print(",");
    Serial.print("w:");
    Serial.print(new_rot.w);
    Serial.print(",");
    Serial.print("a:");
    Serial.print(new_rot.x);
    Serial.print(",");
    Serial.print("b:");
    Serial.print(new_rot.y);
    Serial.print(",");
    Serial.print("c:");
    Serial.print(new_rot.z);
    Serial.print(",");
    Serial.print("mov:");
    Serial.println(MomentarilyStationary(new_accel));
  }
  
  //TODO: update last_corrected at the end
}

/// Puts IMU into sleep mode.
void SleepIMU() {
  Wire.beginTransmission(IMU_ADDRESS);  // Start communication with MPU6050
  Wire.write(0x6B);                     // Talk to the register 6B
  Wire.write(0x40);                     // Set sleep bit to 1
  Wire.endTransmission(true);           //end the transmission
}
