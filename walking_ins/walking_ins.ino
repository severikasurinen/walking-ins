#include "quaternion_math.h"
#include "ble_manager.h"
#include "imu_manager.h"

#define POWER_LED 8

uint32_t last_action = 0, current_value = 0;
uint16_t sleep_time = 600;  // DO NOT SET BELOW 15 SECONDS!!!

void setup() {
  delay(3000);
  Serial.begin(115200);
  Serial.println();
  SetupBLE();
  SetupIMU();

  pinMode(POWER_LED, OUTPUT);
  digitalWrite(POWER_LED, HIGH);
}

void loop() {
  if (device_connected) {
    delay(100);
    current_value = millis();
    SetData(current_value);
    last_action = millis();
  } else if (millis() - last_action > sleep_time * 1000) {
    digitalWrite(POWER_LED, LOW);
    SleepIMU();
    esp_deep_sleep_start();  // Power off to save battery
  } else {
    delay(100);
    UpdateIMU();
  }
}
