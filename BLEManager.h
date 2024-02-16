#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "2e5dc756-78bd-405c-bb72-9641a6848842"
#define CONTROL_CHARACTERISTIC_UUID "0cf0cef9-ec1a-495a-a007-4de6037a303b"
#define DATA_CHARACTERISTIC_UUID "a20eebe5-dfbf-4428-bb7b-84e40d102681"

extern bool device_connected = false;

char advertising_name[] = "ATA_S";
extern BLECharacteristic controlCharacteristic(CONTROL_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);
BLEDescriptor controlDescriptor(BLEUUID((uint16_t)0x2902));
extern BLECharacteristic dataCharacteristic(DATA_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor dataDescriptor(BLEUUID((uint16_t)0x2903));

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    device_connected = true;
    Serial.println("Device connected.");
  };
  void onDisconnect(BLEServer* pServer) {
    device_connected = false;
    Serial.println("Device disconnected.");
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
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
};

extern void SetupBLE() {
  BLEDevice::init(advertising_name);
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
  Serial.print(advertising_name);
  Serial.println("'.");
}

void SetData(uint32_t in_data) {
    dataCharacteristic.setValue(in_data);
    dataCharacteristic.notify();
}