#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h> // ΠΡΟΣΘΗΚΗ: Απαραίτητο για το Descriptor

// Βle server defined
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLECharacteristic *pCharacteristic;

// Oled display defined
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ADS115 defined
Adafruit_ADS1115 ads;
#define I2C_SDA  6
#define I2C_SCL  7

// ─── ΧΡΟΝΙΣΜΟΣ & ΔΕΙΓΜΑΤΟΛΗΨΙΑ (Αλλαγή για MIT-BIH) ─────────
#define FS 360UL // ΑΛΛΑΓΗ: 360Hz για το ML μοντέλο σου
#define SAMPLE_PERIOD_US (1000000UL / FS)   // ~2777 µs
unsigned long last_sample_us = 0;
unsigned long last_oled_update = 0;

// ============================================================
//  IIR Biquad Φίλτρο (Για τα 50Hz)
// ============================================================
struct Biquad {
  float b0, b1, b2;
  float a1, a2;
  float x1, x2;
  float y1, y2;
};

Biquad notch50;

void biquadInit(Biquad* f, float b0, float b1, float b2, float a1, float a2) {
  f->b0=b0; f->b1=b1; f->b2=b2;
  f->a1=a1; f->a2=a2;
  f->x1=f->x2=f->y1=f->y2=0.0f;
}

float biquadProcess(Biquad* f, float x) {
  float y = f->b0*x + f->b1*f->x1 + f->b2*f->x2 - f->a1*f->y1 - f->a2*f->y2;
  f->x2=f->x1; f->x1=x;
  f->y2=f->y1; f->y1=y;
  return y;
}

// ============================================================
//  Μεταβλητές Αλγορίθμου QRS
// ============================================================
float dc_offset = 0.0f;
float last_notched = 0.0f;
float threshold = 5000.0f; 

unsigned long last_peak_time = 0;
float bpm = 0.0f;

// ============================================================
//  Setup
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);

  // Αρχικοποίηση Οθόνης
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Σφάλμα: Η οθόνη OLED δεν βρέθηκε!"));
    while(true);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println("ECG Start..");
  display.display();

  // Αρχικοποίηση ADS1115
  if (!ads.begin(0x48)) {
    Serial.println("Σφάλμα: Το ADS1115 δεν βρέθηκε!");
    while (true) delay(1000);
  }
  ads.setGain(GAIN_TWO);             
  ads.setDataRate(RATE_ADS1115_860SPS);

  // ΑΛΛΑΓΗ: Νέοι συντελεστές για Φίλτρο 50Hz σε δειγματοληψία 360Hz
  biquadInit(&notch50, 0.9535f, -1.2258f, 0.9535f, -1.2213f, 0.9025f);

  Serial.println("Εκκίνηση Συστήματος...");
  last_sample_us = micros();
  
  // --- BLE SETUP ---
  Serial.println("Starting BLE work!");
  BLEDevice::init("EcG");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // ΔΙΟΡΘΩΣΗ: Αφαιρέθηκε το "BLECharacteristic *" για να πάρει τιμή η Global μεταβλητή. Προστέθηκε το σύμβολο |
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE | 
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
                                       
  pCharacteristic->addDescriptor(new BLE2902()); // ΔΙΟΡΘΩΣΗ: Προστέθηκε ερωτηματικό
  
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  Serial.println("Esp is advertising");
}

// ============================================================
//  Loop
// ============================================================
void loop() {
  unsigned long now_us = micros();
  unsigned long now_ms = millis();

  // Έλεγχος δειγματοληψίας (πλέον ελέγχει για ~2.77ms)
  if (now_us - last_sample_us >= SAMPLE_PERIOD_US) {
    last_sample_us += SAMPLE_PERIOD_US;

    // --- ΑΝΑΓΝΩΣΗ & ΦΙΛΤΡΑΡΙΣΜΑ ---
    int16_t raw = ads.readADC_SingleEnded(0);
    
    dc_offset = dc_offset * 0.975f + (float)raw * 0.025f;
    float centered = (float)raw - dc_offset;
    float notched = biquadProcess(&notch50, centered);

    // --- ΑΠΟΣΤΟΛΗ BLE ---
    // Μετατρέπουμε το float σε String για ασφαλή και εύκολη αποστολή μέσω BLE
    String bleMessage = String(notched, 2); 
    pCharacteristic->setValue(bleMessage.c_str()); // ΔΙΟΡΘΩΣΗ: Σωστό format και ερωτηματικό
    pCharacteristic->notify();

    // --- ΑΛΓΟΡΙΘΜΟΣ ΑΝΙΧΝΕΥΣΗΣ QRS ---
    float derivative = notched - last_notched;
    last_notched = notched;
    float squared = derivative * derivative;

    threshold = threshold * 0.995f; 
    if (threshold < 100.0f) {
      threshold = 100.0f; 
    }

    if (squared > threshold && (now_ms - last_peak_time) > 250) {
      float rr_interval = (float)(now_ms - last_peak_time);
      float current_bpm = 60000.0f / rr_interval;
      
      if (current_bpm > 40 && current_bpm < 180) {
        bpm = current_bpm; 
        Serial.print("BPM: ");
        Serial.println((int)bpm);
      }
      
      last_peak_time = now_ms;
      threshold = squared; 
    }
  }

  // Ανανέωση της Οθόνης OLED
  if (now_ms - last_oled_update > 1000) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print("Heart Rate");
    
    display.setTextSize(3);
    display.setCursor(20, 35);
    display.print((int)bpm);
    display.setTextSize(2);
    display.print(" bpm");
    
    display.display();
    last_oled_update = now_ms;
  }
}