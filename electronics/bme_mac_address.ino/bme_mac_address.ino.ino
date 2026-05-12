#include <BLEDevice.h>

void setup() {
  Serial.begin(115200);
  
  // Μικρή καθυστέρηση για να προλάβεις να ανοίξεις το Serial Monitor
  delay(2000); 

  Serial.println("Αναζήτηση BLE MAC Address...");

  // Αρχικοποίηση της μονάδας Bluetooth
  BLEDevice::init("ESP32_Test");

  // Λήψη και εκτύπωση της διεύθυνσης
  Serial.print("Η BLE MAC Address του ESP32 είναι: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());
}

void loop() {
  // Δεν χρειάζεται να κάνει κάτι άλλο
}