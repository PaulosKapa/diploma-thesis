#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h> // REQUIRED for the notify descriptor

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

// Variables to track if the phone is connected
bool deviceConnected = false;
bool oldDeviceConnected = false;

uint32_t i = 0;
// int array[3600] = {}; // Kept for your future batching

// Callback class to check if Flutter is actually connected
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  
  BLEDevice::init("EcG"); 
  // --- ADD THESE TWO LINES ---
  Serial.print("My exact MAC Address is: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());
  // ---------------------------
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks()); // Attach the connection tracker
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // FIX: Removed the "BLECharacteristic *" type declaration so it updates the global variable
  // FIX: Added the missing bitwise OR '|' before PROPERTY_NOTIFY
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                    
  // FIX: Added missing semicolon and ensured BLE2902.h is included above
  pCharacteristic->addDescriptor(new BLE2902());
  
  pService->start();
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true); 
  pAdvertising->setMinPreferred(0x06);  
  pAdvertising->setMinPreferred(0x12);
  
  BLEDevice::startAdvertising();
  Serial.println("Esp is advertising");
}

void loop() {
  // FIX: Only notify if Flutter is actually connected.
  if (deviceConnected) {
      i += 1;
      
      // FIX: Added missing semicolon. 
      // Passing it as a clean 4-byte array prevents endianness/size issues in Flutter
      pCharacteristic->setValue((uint8_t*)&i, 4);
      pCharacteristic->notify();
      
      delayMicroseconds(2777); // ~360 Hz
  }
  
  // Handle disconnection cleanly so you can reconnect without restarting the ESP32
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); 
      pServer->startAdvertising(); 
      Serial.println("Restarted advertising");
      oldDeviceConnected = deviceConnected;
  }
  
  // Handle connecting
  if (deviceConnected && !oldDeviceConnected) {
      oldDeviceConnected = deviceConnected;
  }
}