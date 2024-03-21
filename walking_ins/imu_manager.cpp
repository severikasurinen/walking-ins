#include "imu_manager.h"

#define IMU_ADDRESS 0x68      // MPU6050 I2C address
#define ACCELERATION_RANGE 1  // 0: 2g, 1: 4g, 2: 8g, 3: 16g
#define GYRO_RANGE 1          // 0: 250, 1: 500, 2: 1000, 3: 2000
#define LOW_PASS_FILTER 0     // 0: disabled, 1-6: increased filtering

#define G_VALUE 9.825         // Helsinki

#define STOP_TIME 200         // Time in ms required to stay still for setting device_moving to false

float gyro_multiplier = 1/(131/pow(2, GYRO_RANGE)); // 16bit to deg/s multiplier, from datasheet
float accel_multiplier = 1;                         // 16bit to m/s^2 multiplier, calculated in setup calibration
Vector gyro_offset;                                 // float offset to deg/s values, calculated in setup calibration
Quaternion rot_offset;                              // Rotation from local to global vectors, calculated in setup calibration

bool device_moving = false;
uint32_t t_stopped = 0;     // Since when device has been still?

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

int16_t* ReadSensor() {
  static int16_t out_data[6];

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

  dt = (micros() - t_last) / 1000000f;  // TODO: Fix overflow issues (rollover every ca. 70 mins)
  t_last = micros();

  if(DEBUG_MODE) {
    Serial.print("x:");
    Serial.print(out_data[0]);
    Serial.print(",");
    Serial.print("y:");
    Serial.print(out_data[1]);
    Serial.print(",");
    Serial.print("z:");
    Serial.print(out_data[2]);
    Serial.print(",");
    Serial.print("a:");
    Serial.print(out_data[3]);
    Serial.print(",");
    Serial.print("b:");
    Serial.print(out_data[4]);
    Serial.print(",");
    Serial.print("c:");
    Serial.println(out_data[5]);
  }
  
  return out_data;
}

tuple<Vector, Quaternion> RawCorrection() {
  Vector out_accel = Vector();
  Quaternion out_rot = Quaternion();

  int16_t in_data[6] = ReadSensor();

  // TODO: Implement corrections using multiplier and offset variables

  return make_tuple(out_accel, out_rot);
}


bool MomentarilyStationary(float tolerance, float g) { //returns true if the norm of the linear acceleration is g within tolerance
  int16_t raw_data[6] = ReadSensor();
  Vector linear_acc = new Vector(raw_data[0], raw_data[1], raw_data[2]); //read linear acceleration
  float norm = sqrt(VectorDot((accel_multiplier*linear_acc), (accel_multiplier*linear_acc))); //multiply raw data by conversion factor to get m/s^2
  if((norm + tolerance < g) || (norm - tolerance > g)) {
    return true;
  } else {
    return false;
  }

}


void SetupCalibration() {
  int16_t raw_data[6] = ReadSensor();
  Vector linear_acc = new Vector(raw_data[0], raw_data[1], raw_data[2]); //read linear acceleration
  Vector angular_vel = new Vector(raw_data[3], raw_data[4], raw_data[5]); //read angular velocities
  Vector normalized_g = new Vector(0,0,1);
  

  Quaternion rot_offset = OffsetQ(linear_acc, normalized_g)
}

void PartialCalibration() {
  int16_t raw_data[6] = ReadSensor();
  // Set speed to 0, reset values to first update of no movement
}

void UpdateIMU() {
  // Use RawCorrection() and last sensor data to calculate new values
}

/// Puts IMU into sleep mode.
void SleepIMU() {
  Wire.beginTransmission(IMU_ADDRESS);  // Start communication with MPU6050
  Wire.write(0x6B);                     // Talk to the register 6B
  Wire.write(0x40);                     // Set sleep bit to 1
  Wire.endTransmission(true);           //end the transmission
}
