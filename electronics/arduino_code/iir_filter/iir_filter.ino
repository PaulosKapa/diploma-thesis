#include <OLED_SSD1306_Chart.h>

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
float last_signal_val = 0.0f;

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

if (now_us - last_sample_us >= SAMPLE_PERIOD_US) {
    // ΣΗΜΑΝΤΙΚΟ: Θέτουμε τον χρόνο στο τώρα, αποτρέποντας τα κολλητά/διπλά διαβάσματα
    last_sample_us = now_us; 

    // // --- ΑΝΑΓΝΩΣΗ ---
    // int16_t raw = ads.readADC_SingleEnded(0);
    
    // // Πιο άμεσο κεντράρισμα του σήματος
    // dc_offset = dc_offset * 0.95f + (float)raw * 0.05f;
    // float signal = (float)raw - dc_offset;

    // // ==========================================
    // //  ΠΡΟΣΘΗΚΗ: SMOOTHING (Εξομάλυνση)
    // // ==========================================
    // static float smoothed_signal = 0.0f;
    
    // // Ο συντελεστής εξομάλυνσης (alpha). 
    // // - 0.3f σημαίνει: 30% εμπιστευόμαστε τη νέα μέτρηση, 70% κρατάμε την παλιά.
    // // - Όσο ΠΙΟ ΜΙΚΡΟ το alpha (π.χ. 0.1f), τόσο ΠΙΟ ΑΠΑΛΟ το σήμα (αλλά έχει καθυστέρηση).
    // // - Όσο ΠΙΟ ΜΕΓΑΛΟ το alpha (π.χ. 0.8f), τόσο πιο πιστό στο αρχικό (αλλά με θόρυβο).
    // float alpha = 0.3f; 
    
    // smoothed_signal = (smoothed_signal * (1.0f - alpha)) + (signal * alpha);
// --- 1. ΑΝΑΓΝΩΣΗ & ΚΕΝΤΡΑΡΙΣΜΑ ---
    int16_t raw = ads.readADC_SingleEnded(0);
    
    dc_offset = dc_offset * 0.95f + (float)raw * 0.05f;
    float centered_signal = (float)raw - dc_offset;

    // --- 2. ΑΦΑΙΡΕΣΗ 50Hz (Notch Filter) ---
    // Εδώ βάζουμε το κεντραρισμένο σήμα και βγάζουμε ένα νέο καθαρό
    float notched_signal = biquadProcess(&notch50, centered_signal);

    // --- 3. ΕΞΟΜΑΛΥΝΣΗ (Smoothing) ---
    static float smoothed_signal = 0.0f;
    float alpha = 0.3f; 
    
    // Εδώ χρησιμοποιούμε το NOTCHED σήμα ως είσοδο για να το εξομαλύνουμε
    smoothed_signal = (smoothed_signal * (1.0f - alpha)) + (notched_signal * alpha);

    // ==========================================
    // ΑΠΟ ΕΔΩ ΚΑΙ ΠΕΡΑ χρησιμοποιείς κανονικά το "smoothed_signal"
    // για το BLE, για το QRS (val = smoothed_signal;) και τα πάντα!
    // ΑΠΟ ΕΔΩ ΚΑΙ ΠΕΡΑ χρησιμοποιείς το "smoothed_signal" 
    // αντί για το "signal" στις αποστολές BLE και στον αλγόριθμο QRS.
    // ==========================================
    
    // Αφαιρούμε προσωρινά το biquadProcess. Τα συνθετικά σήματα 
    // από γεννήτριες παραμορφώνονται εύκολα από τα φίλτρα.
    //smoothed_signal = biquadProcess(&notch50, smoothed_signal);
    // --- ΑΠΟΣΤΟΛΗ BLE & SERIAL (72Hz) ---
    static int output_counter = 0;
    output_counter++;
    if (output_counter >= 5) {
      output_counter = 0;
      String bleMessage = String(smoothed_signal, 1); 
      pCharacteristic->setValue(bleMessage.c_str()); 
      pCharacteristic->notify();
    }

   // --- ΑΠΛΟΣ ΑΛΓΟΡΙΘΜΟΣ QRS (ΜΕ ΣΤΑΘΕΡΟ ΟΡΙΟ) ---
    
    float val = smoothed_signal; 
    
    // Το "σκληρό" όριο σου (αγνοεί τα πάντα κάτω από 18.000)
    float my_hard_threshold = 18000.0f; 

    // Ελέγχουμε απλά αν το σήμα πέρασε το όριο
    if (val > my_hard_threshold) {
      
      // Blanking 300ms για να μην μετρήσουμε το ίδιο R-peak δυο φορές
      if ((now_ms - last_peak_time) > 300) {
        float rr_interval = (float)(now_ms - last_peak_time);
        float current_bpm = 60000.0f / rr_interval;
        
        // Λογικά όρια BPM
        if (current_bpm > 40 && current_bpm < 180) {
          if (bpm == 0.0f) {
              bpm = current_bpm;
          } else {
              bpm = (bpm * 0.7f) + (current_bpm * 0.3f); // Ομαλοποίηση
          }
        }
        last_peak_time = now_ms;
      }
    }
    Serial.print(val);
    Serial.print(",");
    Serial.println(my_hard_threshold);
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