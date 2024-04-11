#include "ble_manager.h"

#define ADVERTISING_NAME "ATA-S"
#define SERVICE_UUID "2e5dc756-78bd-405c-bb72-9641a6848842"
#define CONTROL_CHARACTERISTIC_UUID "0cf0cef9-ec1a-495a-a007-4de6037a303b"
#define DATA_CHARACTERISTIC_UUID "a20eebe5-dfbf-4428-bb7b-84e40d102681"

char device_state = 0;
uint32_t last_action = 0;

BLECharacteristic controlCharacteristic(CONTROL_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);
BLEDescriptor controlDescriptor(BLEUUID((uint16_t)0x2902));
BLECharacteristic dataCharacteristic(DATA_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor dataDescriptor(BLEUUID((uint16_t)0x2903));

void MyServerCallbacks::onConnect(BLEServer *pServer) {
  device_state = 1;
  Serial.println("Device connected.");
};
void MyServerCallbacks::onDisconnect(BLEServer *pServer) {
  device_state = 0;
  last_action = millis();
  Serial.println("Device disconnected.");
}

void MyCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
  std::string value = pCharacteristic->getValue();
  if (value.length() > 0) {
    Serial.println("*********");
    Serial.print("New value: ");
    for (int i = 0; i < value.length(); i++)
      Serial.print(value[i]);
    Serial.println();
    Serial.println("*********");
  }
}

void SetupBLE() {
  BLEDevice::init(ADVERTISING_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pService->addCharacteristic(&controlCharacteristic);
  controlCharacteristic.setCallbacks(new MyCallbacks());
  controlCharacteristic.setValue("init");

  pService->addCharacteristic(&dataCharacteristic);
  uint32_t initVal = 0;
  dataCharacteristic.setValue(initVal);

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.print("Started advertising BLE as '");
  Serial.print(ADVERTISING_NAME);
  Serial.println("'.");
}

void SendData(uint8_t* data, size_t length) {
  dataCharacteristic.setValue(data, length);
  dataCharacteristic.notify();
}
