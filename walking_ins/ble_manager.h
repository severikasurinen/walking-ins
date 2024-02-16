#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "Arduino.h"

#define ADVERTISING_NAME              "ATA-S"
#define SERVICE_UUID                  "2e5dc756-78bd-405c-bb72-9641a6848842"
#define CONTROL_CHARACTERISTIC_UUID   "0cf0cef9-ec1a-495a-a007-4de6037a303b"
#define DATA_CHARACTERISTIC_UUID      "a20eebe5-dfbf-4428-bb7b-84e40d102681"

extern bool device_connected;

extern BLECharacteristic controlCharacteristic;
extern BLEDescriptor controlDescriptor;
extern BLECharacteristic dataCharacteristic;
extern BLEDescriptor dataDescriptor;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer);
  void onDisconnect(BLEServer* pServer);
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic);
};

extern void SetupBLE();

void SetData(uint32_t in_data);

#endif
