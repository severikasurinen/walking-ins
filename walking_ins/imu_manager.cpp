#include "imu_manager.h"

#define DEBUG_MODE false

#define IMU_ADDRESS 0x68      // MPU6050 I2C address
#define ACCELERATION_RANGE 2  // 0: 2g, 1: 4g, 2: 8g, 3: 16g
#define GYRO_RANGE 2          // 0: 250, 1: 500, 2: 1000, 3: 2000
#define LOW_PASS_FILTER 0     // 0: disabled, 1-6: increased filtering

#define G_VALUE 9.825         // Helsinki

#define SETUP_ITERATION 300

#define STOP_ACCELERATION_MAX 5.0 // Maximum acceleration norm in m/s^2 for setting device_moving to false
#define STOP_GYRO_MAX 100 // Maximum gyro norm in deg/s for setting device_moving to false
#define STOP_TIME_MIN 50 // Time in ms required to stay still for setting device_moving to false
#define MOVE_TIME_MIN 25 // Time in ms required to move for setting device_moving to true

#define CALIB_ACCELERATION_MAX 0.3 // Maximum acceleration norm in m/s^2 for starting partial calibration
#define CALIB_GYRO_MAX 25 // Maximum gyro norm in deg/s for starting partial calibration
#define CALIB_GYRO_RANGE_MAX 1.0 // Maximum gyro norm range in deg/s for partial calibration
#define CALIB_INTERVAL_MIN 500 // Minimum interval between partial calibrations, unless new conditions are better than last calibration's
#define CALIB_TIME_MIN 40   // Time in ms required to perform partial calibration


float gyro_multiplier = 1.0 / (131/pow(2, GYRO_RANGE)) * 1.0; // 16bit to deg/s multiplier, from datasheet, last multiplier tested empirically
float accel_multiplier = 1.0;                         // 16bit to m/s^2 multiplier, calculated in setup calibration
Vector gyro_offset = Vector();                                 // float offset to deg/s values, calculated in setup calibration
Quaternion rot_offset = Quaternion();

Vector acceleration = Vector();
Vector angular_velocity = Vector();

Vector velocity = Vector();
Vector position = Vector();
Quaternion orientation = Quaternion();

unsigned long t_last = 0;

bool device_moving = false;
uint32_t t_stopped = 0;     // Since when has device been still?
uint32_t t_moved = 0;     // Since when has device been moving?
Vector stopped_position = Vector();

uint32_t t_calib = 0;   // Since when has device been calibrating?
float calib_gyro_max = 0;  // Maximum gyro in deg/s of current calibration
float calib_gyro_min = 0;  // Minimum gyro in deg/s of current calibration
uint32_t t_last_calib = 0;  // When did last calibration finish?
float last_calib_min = 0;  // calib_gyro_min of last calibration
uint16_t avg_iterations = 0;
Vector avg_acceleration = Vector();

uint8_t print_iters = 0;


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

uint8_t MomentarilyStationary(float accel_norm, float gyro_norm) {
  //returns true if the norm of the linear acceleration is g within tolerance

  if(accel_norm <= CALIB_ACCELERATION_MAX && gyro_norm <= CALIB_GYRO_MAX && calib_gyro_max - calib_gyro_min <= CALIB_GYRO_RANGE_MAX) {
    return 2;
  }
  else if(accel_norm <= STOP_ACCELERATION_MAX && gyro_norm <= STOP_GYRO_MAX) {
    return 1;
  }
  else {
    return 0;
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
  rot_offset = GetRotationBetween(linear_acc, normalized_g);

  Serial.println("Setup calibration done.");

  // Reset all necessary values
  acceleration = Vector();
  angular_velocity = Vector();
  velocity = Vector();
  position = Vector();
  orientation = Quaternion();
  t_last = 0;
  device_moving = false;
  t_stopped = 0;
  t_moved = 0;
  stopped_position = Vector();
  t_calib = 0;
  calib_gyro_max = 0;
  calib_gyro_min = 0;
  t_last_calib = 0;
  last_calib_min = 0;
  avg_iterations = 0;
  avg_acceleration = Vector();
  print_iters = 0;
}

void PartialCalibration(Vector avg_accel) {
  // Reset orientation to align with ground
  Vector euler_yaw = GetEuler(orientation);
  euler_yaw = Vector(0, 0, euler_yaw.z);
  orientation = euler_yaw.ToQuaternion();

  // Calculate new rot_offset
  Vector normalized_g = Vector(0, 0, 1.0);
  rot_offset = GetRotationBetween(avg_accel.GetRotated(orientation), normalized_g); // Fix rotation to align with gravity, while retaining yaw information.
}

void SetMoving(bool new_moving) {
  if(new_moving) {
    device_moving = true;
  }
  else {
    device_moving = false;
        
    acceleration = Vector();
    velocity = Vector();
    position = stopped_position;
  }
}

void UpdateIMU() {
  // Use RawCorrection() and last sensor data to calculate new values

  std::array<float, 6> new_data = RawCorrection();
  Vector new_accel = Vector(new_data[0], new_data[1], new_data[2]);
  Vector new_gyro = Vector(new_data[3], new_data[4], new_data[5]);

  float accel_norm = abs(G_VALUE - new_accel.GetMagnitude());
  float gyro_norm = new_gyro.GetMagnitude();
  uint8_t cur_stationary = MomentarilyStationary(accel_norm, gyro_norm);
  
  if(cur_stationary > 0) {
    t_moved = 0;

    if(device_moving) {
      if(t_stopped == 0) {
        stopped_position = position;
        t_stopped = millis();
      }
      else if(millis() - t_stopped >= STOP_TIME_MIN) {
          SetMoving(false);
      }
    }
  }
  else {
    t_stopped = 0;

    if(!device_moving) {
      if(t_moved == 0) {
        t_moved = millis();
      }
      else if(millis() - t_moved >= MOVE_TIME_MIN) {
        SetMoving(true);
      }
    }
  }

  if(cur_stationary == 2 && calib_gyro_max - calib_gyro_min <= CALIB_GYRO_RANGE_MAX) {
    if(t_calib == 0) {
      if(millis() - t_last_calib >= CALIB_INTERVAL_MIN || gyro_norm < last_calib_min) {
        t_calib = millis();

        calib_gyro_max = gyro_norm;
        calib_gyro_min = gyro_norm;
      }
    }
    else {
      avg_iterations++;
      avg_acceleration = avg_acceleration + new_accel;

      calib_gyro_max = max(gyro_norm, calib_gyro_max);
      calib_gyro_min = min(gyro_norm, calib_gyro_min);
    }
  }
  else {
    // Perform orientation calibration if enough time spent calibrating
    if(t_calib > 0 && millis() - t_calib >= CALIB_TIME_MIN) {
      PartialCalibration(avg_acceleration / avg_iterations);
      last_calib_min = calib_gyro_min;
      t_last_calib = millis();
    }
    t_calib = 0;
    calib_gyro_max = 0;
    calib_gyro_min = 0;
    avg_iterations = 0;
    avg_acceleration = Vector();
  }

  // Measure time spent between updates
  float dt = (micros() - t_last) / 1000000.0;
  if (micros() < t_last) {
    dt = (ULONG_MAX - t_last + micros()) / 1000000.0;
  }
  t_last = micros();
  if (dt > 0.005) {
    Serial.print("Only ");
    Serial.print(1 / dt);
    Serial.println(" Hz!");
  }

  if(cur_stationary < 2) {
    // Update orientation based on gyro data
    Vector avg_gyro = angular_velocity.Average(new_gyro);
    orientation = (avg_gyro * dt).ToQuaternion().GetProduct(orientation);
    angular_velocity = new_gyro;
  }
  else {
    angular_velocity = Vector();
  }

  if (device_moving) {
    // Rotate acceleration to global orientation
    new_accel = new_accel.GetRotated(orientation.GetProduct(rot_offset));
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
  }

  if (DEBUG_MODE) {
    if (print_iters > 100) {
      Serial.print("x:");
      Serial.print(acceleration.x);
      Serial.print(",");
      Serial.print("y:");
      Serial.print(acceleration.y);
      Serial.print(",");
      Serial.print("z:");
      Serial.print(acceleration.z);
      Serial.print(",");

      Vector euler_rot = GetEuler(orientation);
      Serial.print("roll:");
      Serial.print(euler_rot.x);
      Serial.print(",");
      Serial.print("pitch:");
      Serial.print(euler_rot.y);
      Serial.print(",");
      Serial.print("yaw:");
      Serial.print(euler_rot.z);
      Serial.print(",");
      Serial.print("mov:");
      Serial.println(device_moving);

      print_iters = 0;
    }
    else {
      print_iters++;
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
