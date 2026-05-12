/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLECharacteristic *pCharacteristic;
int i = 0;
int array[3600] = {};
void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  //name the device
  BLEDevice::init("EcG");
  //set the ecg device as a server
  BLEServer *pServer = BLEDevice::createServer();
  //creat a service for the ble server
  BLEService *pService = pServer->createService(SERVICE_UUID);
  //define the characteristics for the service
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                         //to send messages without flutter asking
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  pCharacteristic->addDescriptor(new BLE2902())
  //start the service
  pService->start();
  //define and start advertising to nearby devices
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  Serial.println("Esp is advertising");
}

void loop() {
  //just for testing
  //we will increase i 360 times per second.
  i+=1;
  pCharacteristic->setValue(i)
  pCharacteristic->notify();
  delayMicroseconds(2777);


}