#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"

extern char device_state;
extern uint32_t last_action;

extern BLECharacteristic controlCharacteristic;
extern BLEDescriptor controlDescriptor;
extern BLECharacteristic dataCharacteristic;
extern BLEDescriptor dataDescriptor;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer);
  void onDisconnect(BLEServer* pServer);
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic);
};

extern void SetupBLE();

extern void SendData(uint32_t in_data);

#endif
