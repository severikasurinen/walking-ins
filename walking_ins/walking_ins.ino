#define DEBUG_MODE true

char device_state = 0;
uint32_t last_action = 0;

#include "quaternion_math.h"
#include "ble_manager.h"
#include "imu_manager.h"

#define SLEEP_TIME 600  // DO NOT SET BELOW 15 SECONDS!!!
#define POWER_LED 8

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
  switch (device_state) {
    case 0: // waiting for BLE
      if (millis() - last_action > SLEEP_TIME * 1000) {
        digitalWrite(POWER_LED, LOW);
        SleepIMU();
        esp_deep_sleep_start();  // Power off to save battery
      }
      else {
        if (DEBUG_MODE) {
          delay(100);
          UpdateIMU();
        } else {
          delay(5000);
        }
      }
      break;
    case 1: // connected, standby
      delay(100);
      SendData(millis());
      break;
    case 2: // active measurement
      if (device_moving) {

      } else {
        SendData(millis());
      }
      break;
    default:
      device_state = 0;
      break;
  }
}
