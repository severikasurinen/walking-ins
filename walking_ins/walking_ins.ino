#define DEBUG_MODE true

#include "quaternion_math.h"
#include "ble_manager.h"
#include "imu_manager.h"

#define SLEEP_TIME 600  // DO NOT SET BELOW 15 SECONDS!!!
#define POWER_LED 8
#define SEND_TIME 1000        // Time in ms between sending data via bluetooth

uint32_t t_sent = 0;     // Since when was the data last sent to the UI?

void setup() {
  delay(3000);
  Serial.begin(115200);
  Serial.println();
  SetupBLE();
  SetupIMU();

  pinMode(POWER_LED, OUTPUT);
  digitalWrite(POWER_LED, HIGH);

  if (DEBUG_MODE) {
    SetupCalibration();
  }
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
          if(t_sent > TIME_SEND) { //send data if enough time has elapsed
    float send_data = send_data[8];
    send_data[0] = (float) millis();
    send_data[1] = position[0];
    send_data[2] = position[1];
    send_data[3] = position[2];
    send_data[4] = rotation[3];
    send_data[5] = rotation[4];
    send_data[6] = rotation[5];
    send_data[7] = rotation[6];
    t_sent = 0;
    size_t length = 8;
    
  } else {
    t_sent = millis();
  }
      }
      break;
    default:
      device_state = 0;
      break;
  }
}
