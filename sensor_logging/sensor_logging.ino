#include "quaternion_math.h"
#include "imu_manager.h"

#define POWER_LED 8

void setup() {
  delay(3000);
  Serial.begin(115200);
  Serial.println();
  SetupIMU();

  pinMode(POWER_LED, OUTPUT);
  digitalWrite(POWER_LED, HIGH);

  SetupCalibration();
}

void loop() {
  UpdateIMU();
}
