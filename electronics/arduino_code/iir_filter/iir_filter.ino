#include <Wire.h>

#define I2C_SDA 6
#define I2C_SCL 7
#define ADS_ADDRESS 0x48

unsigned long last_sample_time = 0;
const unsigned long sample_interval = 4000; // 250 Hz

float dc_offset = 0.0;

// Πίνακας για τον Κινητό Μέσο Όρο (5 δείγματα)
const int NUM_SAMPLES = 5;
float buffer50Hz[NUM_SAMPLES] = {0};
int buffer_index = 0;

// Η νέα, πανέξυπνη συνάρτηση φίλτρου
float apply50HzMovingAverage(float new_sample) {
  // 1. Αποθηκεύουμε το νέο δείγμα στη μνήμη
  buffer50Hz[buffer_index] = new_sample;
  
  // 2. Προχωράμε τον δείκτη (κυκλική μνήμη)
  buffer_index++;
  if (buffer_index >= NUM_SAMPLES) {
    buffer_index = 0;
  }
  
  // 3. Βρίσκουμε τον μέσο όρο των 5 τελευταίων δειγμάτων
  float sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += buffer50Hz[i];
  }
  return sum / (float)NUM_SAMPLES;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Αυξάνουμε την ταχύτητα του I2C στα 400kHz για να μειώσουμε το Jitter!
  Wire.setClock(400000); 

  Wire.beginTransmission(ADS_ADDRESS);
  Wire.write(0x01); 
  Wire.write(0x40); 
  Wire.write(0xE3); // 860 SPS
  byte error = Wire.endTransmission();
  
  if(error != 0) {
    Serial.println("Σφάλμα σύνδεσης με το ADS1115!");
    while(1);
  }

  Wire.beginTransmission(ADS_ADDRESS);
  Wire.write(0x00); 
  Wire.endTransmission();
}

void loop() {
  unsigned long current_time = micros();

  // Χρησιμοποιούμε απλό '=' για να μην δημιουργούνται "εκρήξεις" (bursts) δειγμάτων
  if (current_time - last_sample_time >= sample_interval) {
    last_sample_time = current_time; 

    Wire.requestFrom(ADS_ADDRESS, 2);
    if (Wire.available() == 2) {
      int16_t raw_adc = (Wire.read() << 8) | Wire.read();
      
      // Κεντράρισμα στο μηδέν
      dc_offset = (dc_offset * 0.99) + ((float)raw_adc * 0.01);
      float centered_signal = (float)raw_adc - dc_offset;
      
      // Εφαρμογή του Κινητού Μέσου Όρου
      float clean_ecg = apply50HzMovingAverage(centered_signal);

      Serial.print(centered_signal);
      Serial.print(",");
      Serial.println(clean_ecg); 
    }
  }
}