#include "imu_manager.h"

#define DEBUG_MODE false

#define IMU_ADDRESS 0x68      // MPU6050 I2C address
#define ACCELERATION_RANGE 1  // 0: 2g, 1: 4g, 2: 8g, 3: 16g
#define GYRO_RANGE 1          // 0: 250, 1: 500, 2: 1000, 3: 2000
#define LOW_PASS_FILTER 0     // 0: disabled, 1-6: increased filtering

#define G_VALUE 9.825         // Helsinki

#define SETUP_ITERATION 200
#define PARTIAL_ITERATION 40
#define MOVING_TOLERANCE 1.2  // in m/s^2
#define STOP_TIME 200         // Time in ms required to stay still for setting device_moving to false


float gyro_multiplier = 1.0 / (131/pow(2, GYRO_RANGE)); // 16bit to deg/s multiplier, from datasheet
float accel_multiplier = 1.0;                         // 16bit to m/s^2 multiplier, calculated in setup calibration
Vector gyro_offset = Vector();                                 // float offset to deg/s values, calculated in setup calibration

Vector stopped_position = Vector();

Vector acceleration = Vector();
Vector angular_velocity = Vector();

Vector velocity = Vector();
Vector position = Vector();
Quaternion rotation = Quaternion();

bool device_moving = false;
uint32_t t_stopped = 0;     // Since when device has been still?
float dt = 0;
unsigned long t_last = 0;

uint8_t idel = 0;



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

bool MomentarilyStationary(Vector in_accel) { //returns true if the norm of the linear acceleration is g within tolerance
  float norm = in_accel.GetMagnitude();
  if(abs(G_VALUE - norm) < MOVING_TOLERANCE) {
    return true;
  } else {
    return false;
  }
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
  rotation = GetRotationBetween(linear_acc, normalized_g);

  Serial.println("Setup calibration done.");
}

void PartialCalibration() {
  acceleration = Vector();
  angular_velocity = Vector();
  velocity = Vector();
  position = stopped_position;

  Vector avg_acc = Vector();
  for (int i = 0; i < PARTIAL_ITERATION; i++) {
    std::array<int16_t, 6> raw_data = ReadSensor();
    avg_acc = avg_acc + (Vector(raw_data[0], raw_data[1], raw_data[2]) / PARTIAL_ITERATION);
    delay(5);
  }
  
  avg_acc = avg_acc.GetRotated(rotation);  // Apply rotation

  Vector normalized_g = Vector(0, 0, 1.0);
  rotation = GetRotationBetween(avg_acc, normalized_g).GetProduct(rotation); // Fix rotation to align with gravity, while retaining yaw information.
}

void UpdateIMU() {
  // Use RawCorrection() and last sensor data to calculate new values

  std::array<float, 6> new_data = RawCorrection();
  Vector new_accel = Vector(new_data[0], new_data[1], new_data[2]);

  if(MomentarilyStationary(new_accel)) {
    //true branch, check if STOP_TIME has elapsed
    if(t_stopped == 0) {
      stopped_position = position;
      t_stopped = millis();
    }
    else if(device_moving) {
      if(millis() < t_stopped) { //overflow branch
        if(UINT_MAX + millis() - t_stopped >= STOP_TIME) {
          device_moving = false;
          PartialCalibration();
        }
      }
      else if(millis() - t_stopped >= STOP_TIME) {
        device_moving = false;
        PartialCalibration();
      }
    }
  }
  else {
    //false branch, device is moving
    device_moving = true;
    t_stopped = 0;

    // Update rotation based on gyro data
    Vector new_gyro = Vector(new_data[3], new_data[4], new_data[5]);
    Vector avg_gyro = angular_velocity.Average(new_gyro);
    rotation = (avg_gyro * dt).ToQuaternion().GetProduct(rotation);

    // Rotate acceleration to global orientation
    new_accel = new_accel.GetRotated(rotation);
    new_accel = Vector(new_accel.x, new_accel.y, new_accel.z - G_VALUE);
    Vector avg_accel = acceleration.Average(new_accel);

    // Integrate acceleration to calculate velocity
    Vector new_vel = velocity + (avg_accel * dt);
    Vector avg_vel = velocity.Average(new_vel);

    // Integrate velocity to calculate position
    position = position + (avg_vel * dt);

    // Update global variables to new values
    velocity = new_vel;
    acceleration = new_accel;
    angular_velocity = new_gyro;
  }

  if (DEBUG_MODE) {
    if (idel > 250) {
      Serial.print("x:");
      Serial.print(position.x);
      Serial.print(",");
      Serial.print("y:");
      Serial.print(position.y);
      Serial.print(",");
      Serial.print("z:");
      Serial.print(position.z);
      Serial.print(",");
      Serial.print("w:");
      Serial.print(rotation.w);
      Serial.print(",");
      Serial.print("a:");
      Serial.print(rotation.x);
      Serial.print(",");
      Serial.print("b:");
      Serial.print(rotation.y);
      Serial.print(",");
      Serial.print("c:");
      Serial.print(rotation.z);
      Serial.print(",");
      Serial.print("mov:");
      Serial.println(device_moving);

      idel = 0;
    }
    else {
      idel++;
    }
  }
}

/// Puts IMU into sleep mode.
void SleepIMU() {
  Wire.beginTransmission(IMU_ADDRESS);  // Start communication with MPU6050
  Wire.write(0x6B);                     // Talk to the register 6B
  Wire.write(0x40);                     // Set sleep bit to 1
  Wire.endTransmission(true);           //end the transmission
}
