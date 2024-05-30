#include "quaternion_math.h"
#include "imu_manager.h"

#define POWER_LED 8   // Pin for power LED

void setup() {
  delay(3000);
  Serial.begin(115200);
  Serial.println();
  SetupIMU();   // Start IMU with selected settings

  // Switch power LED on
  pinMode(POWER_LED, OUTPUT);
  digitalWrite(POWER_LED, HIGH);

  SetupCalibration(); // Start initial calibration
}

void loop() {
  UpdateIMU();  // Read new values from IMU
}
