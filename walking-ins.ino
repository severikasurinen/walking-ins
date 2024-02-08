#include <Wire.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define IMU_ADDRESS 0x68      // MPU6050 I2C address
#define ACCELERATION_RANGE 1  // 0: 2g, 1: 4g, 2: 8g, 3: 16g
#define GYRO_RANGE 1          // 0: 250, 1: 500, 2: 1000, 3: 2000
#define LOW_PASS_FILTER 0     // 0: disabled, 1-6: increased filtering

#define SERVICE_UUID        "2e5dc756-78bd-405c-bb72-9641a6848842"
#define CONTROL_CHARACTERISTIC_UUID "0cf0cef9-ec1a-495a-a007-4de6037a303b"
#define DATA_CHARACTERISTIC_UUID "a20eebe5-dfbf-4428-bb7b-84e40d102681"

char advertising_name[] = "ATA_S";
BLECharacteristic controlCharacteristic(CONTROL_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);
BLEDescriptor controlDescriptor(BLEUUID((uint16_t)0x2902));
BLECharacteristic dataCharacteristic(DATA_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor dataDescriptor(BLEUUID((uint16_t)0x2903));

bool device_connected = false;
uint32_t last_action = 0, current_value = 0;
uint16_t sleep_time = 600; // DO NOT SET BELOW 15 SECONDS!!!

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    device_connected = true;
    Serial.println("Device connected.");
  };
  void onDisconnect(BLEServer* pServer) {
    device_connected = false;
    Serial.println("Device disconnected.");

    last_action = millis();
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

void SetupBLE() {
  BLEDevice::init(advertising_name);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pService->addCharacteristic(&controlCharacteristic);
  controlCharacteristic.setCallbacks(new MyCallbacks());
  controlCharacteristic.setValue("init");

  pService->addCharacteristic(&dataCharacteristic);
  dataCharacteristic.setValue(current_value);

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

void SetupIMU() {
  Wire.begin();                         // Initialize comunication
  Wire.setClock(400000);                // 400kHz I2C clock. Comment this line if having compilation difficulties

  Wire.beginTransmission(IMU_ADDRESS);  // Start communication with MPU6050
  Wire.write(0x6B);                     // Talk to the register 6B
  Wire.write(0x00);                     // Reset device
  Wire.endTransmission(true);           //end the transmission

  // Configure Digital Low Pass Filter - default disabled
  Wire.beginTransmission(IMU_ADDRESS);
  Wire.write(0x1A);             // Talk to the DLPF_CFG register (1A hex)
  Wire.write(LOW_PASS_FILTER);  // Set the register bits
  Wire.endTransmission(true);

  // Configure Gyro Sensitivity - default +/- 250deg/s
  Wire.beginTransmission(IMU_ADDRESS);
  Wire.write(0x1B);             // Talk to the GYRO_CONFIG register (1B hex)
  Wire.write(GYRO_RANGE << 3);  // Set the register bits
  Wire.endTransmission(true);

  // Configure Accelerometer Sensitivity - default +/- 2g
  Wire.beginTransmission(IMU_ADDRESS);
  Wire.write(0x1C);                     //Talk to the ACCEL_CONFIG register (1C hex)
  Wire.write(ACCELERATION_RANGE << 3);  //Set the register bits
  Wire.endTransmission(true);

  Serial.println("MPU6050 setup done.");
}

void UpdateIMU() {
  Wire.beginTransmission(IMU_ADDRESS);
  Wire.write(0x3B);  // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(IMU_ADDRESS, 14, true);  // Read 14 registers total, each axis value is stored in 2 registers

  int16_t raw_data[7];

  raw_data[0] = Wire.read() << 8 | Wire.read();  // X-axis acceleration data
  raw_data[1] = Wire.read() << 8 | Wire.read();  // Y-axis acceleration data
  raw_data[2] = Wire.read() << 8 | Wire.read();  // Z-axis acceleration data

  raw_data[6] = Wire.read() << 8 | Wire.read();  // Temperature value in Celsius

  raw_data[3] = Wire.read() << 8 | Wire.read();  // X-axis gyro data
  raw_data[4] = Wire.read() << 8 | Wire.read();  // Y-axis gyro data
  raw_data[5] = Wire.read() << 8 | Wire.read();  // Z-axis gyro data

  Serial.print("x:");
  Serial.print(raw_data[0]);
  Serial.print(",");
  Serial.print("y:");
  Serial.print(raw_data[1]);
  Serial.print(",");
  Serial.print("z:");
  Serial.print(raw_data[2]);
  Serial.print(",");
  Serial.print("a:");
  Serial.print(raw_data[3]);
  Serial.print(",");
  Serial.print("b:");
  Serial.print(raw_data[4]);
  Serial.print(",");
  Serial.print("c:");
  Serial.println(raw_data[5]);
  /*
  for(int i = 0; i < 7; i++) {
    Serial.print(raw_data[i]);
    if(i < 6) {
      Serial.print(" | ");
    }
    else {
      Serial.println();
    }
  }
  */
}

void setup() {
  delay(3000);
  Serial.begin(115200);
  Serial.println();
  SetupBLE();
  SetupIMU();
}

void loop() {
  if(device_connected) {
    delay(100);
    current_value = millis();
    dataCharacteristic.setValue(current_value);
    dataCharacteristic.notify();
  }
  else if(millis() - last_action > sleep_time * 1000) {
    esp_deep_sleep_start(); // Power off to save battery
  }
  else {
    delay(100);
    UpdateIMU();
  }
}