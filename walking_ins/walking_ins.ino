#define DEBUG_MODE false

#include "quaternion_math.h"
#include "ble_manager.h"
#include "imu_manager.h"

#define SLEEP_TIME 600  // DO NOT SET BELOW 15 SECONDS!!!
#define POWER_LED 8     // Pin for power LED
#define SEND_TIME 500   // Time in ms between sending data via bluetooth

uint32_t t_sent = 0;    // Since when was the data last sent to the UI?

void setup() {
  delay(5000);
  Serial.begin(115200);
  Serial.println();
  SetupBLE();
  SetupIMU();   // Start IMU with selected settings

  // Switch power LED on
  pinMode(POWER_LED, OUTPUT);
  digitalWrite(POWER_LED, HIGH);
}

void loop() {
  switch (device_state) {
    case 0: // Waiting for BLE

      // Power off to save battery after disconnected for selected time
      if (millis() - last_action > SLEEP_TIME * 1000) {
        digitalWrite(POWER_LED, LOW);
        SleepIMU();
        esp_deep_sleep_start();
      }
      else {
        if (DEBUG_MODE) {
          device_state = 2;
        }
        delay(5000);
      }
      break;
    case 1: // Connected, standby
      delay(100);
      break;
    case 2: // Starting measurement
      uint8_t send_data[1];
      send_data[0] = 0;
      SendData(send_data, 1); // Reset sent data
      SetupCalibration();   // Start initial calibration
      device_state = 3;
      break;
    case 3: // Active measurement
      UpdateIMU();  // Read new values from IMU

      // Send data if enough time has elapsed
      if(!device_moving && millis() - t_sent >= SEND_TIME) {
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
        
        SendData(send_data, 32);  // Send sensor data to receiver
        t_sent = millis();
      }
      break;
    default:
      device_state = 0;
      break;
  }
}
