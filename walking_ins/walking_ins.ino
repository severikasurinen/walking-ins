#define DEBUG_MODE false

#include "quaternion_math.h"
#include "ble_manager.h"
#include "imu_manager.h"

#define SLEEP_TIME 600  // DO NOT SET BELOW 15 SECONDS!!!
#define POWER_LED 8
#define SEND_TIME 500        // Time in ms between sending data via bluetooth

uint32_t t_sent = 0;     // Since when was the data last sent to the UI?

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
          device_state = 2;
        }
        delay(5000);
      }
      break;
    case 1: // connected, standby
      delay(100);
      break;
    case 2: // starting measurement
      SetupCalibration();
      device_state = 3;
      break;
    case 3: // active measurement
      UpdateIMU();

      if(!device_moving && millis() - t_sent >= SEND_TIME) { //send data if enough time has elapsed
        uint8_t send_data[32];

        uint8_t *temp_arr;
        
        unsigned long ul = millis();
        temp_arr = reinterpret_cast<uint8_t*>(&ul);
        for (int i = 0; i < 4; i++) {
          send_data[i] = temp_arr[i];
        }
        float f = position.x;
        temp_arr = reinterpret_cast<uint8_t*>(&f);
        for (int i = 0; i < 4; i++) {
          send_data[4+i] = temp_arr[i];
        }
        f = position.y;
        temp_arr = reinterpret_cast<uint8_t*>(&f);
        for (int i = 0; i < 4; i++) {
          send_data[8+i] = temp_arr[i];
        }
        f = position.z;
        temp_arr = reinterpret_cast<uint8_t*>(&f);
        for (int i = 0; i < 4; i++) {
          send_data[12+i] = temp_arr[i];
        }
        f = orientation.w;
        temp_arr = reinterpret_cast<uint8_t*>(&f);
        for (int i = 0; i < 4; i++) {
          send_data[16+i] = temp_arr[i];
        }
        f = orientation.x;
        temp_arr = reinterpret_cast<uint8_t*>(&f);
        for (int i = 0; i < 4; i++) {
          send_data[20+i] = temp_arr[i];
        }
        f = orientation.y;
        temp_arr = reinterpret_cast<uint8_t*>(&f);
        for (int i = 0; i < 4; i++) {
          send_data[24+i] = temp_arr[i];
        }
        f = orientation.z;
        temp_arr = reinterpret_cast<uint8_t*>(&f);
        for (int i = 0; i < 4; i++) {
          send_data[28+i] = temp_arr[i];
        }
        
        SendData(send_data, 32);
        t_sent = millis();
      }
      break;
    default:
      device_state = 0;
      break;
  }
}
