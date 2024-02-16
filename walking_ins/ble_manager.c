#include "ble_manager.h"

bool device_connected = false;

BLECharacteristic controlCharacteristic(CONTROL_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);
BLEDescriptor controlDescriptor(BLEUUID((uint16_t)0x2902));
BLECharacteristic dataCharacteristic(DATA_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor dataDescriptor(BLEUUID((uint16_t)0x2903));

void MyServerCallbacks::onConnect(BLEServer *pServer) {
  device_connected = true;
  Serial.println("Device connected.");
};
void MyServerCallbacks::onDisconnect(BLEServer *pServer) {
  device_connected = false;
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
  dataCharacteristic.setValue(uint32_t(0));

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

void SetData(uint32_t in_data) {
  dataCharacteristic.setValue(in_data);
  dataCharacteristic.notify();
}
