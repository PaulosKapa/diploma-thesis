#include <OLED_SSD1306_Chart.h>
//libraries used for i2c communication
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//libraries used for ble communication
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

//define a ble server
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
//create a BLEcharacteristic object
BLECharacteristic *pCharacteristic;

//define attributes of oled display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//create an ads1115 object and define the i2c pins
Adafruit_ADS1115 ads;
#define I2C_SDA  6
#define I2C_SCL  7

//define the frequenxy sampling as 360 Hz to match the MIT-BIH database
#define FS 360UL // ΑΛΛΑΓΗ: 360Hz για το ML μοντέλο σου
#define SAMPLE_PERIOD_US (1000000UL / FS)
unsigned long last_sample_us = 0;
unsigned long last_oled_update = 0;

//create a struct for the IIR filter 
struct Biquad {
  float b0, b1, b2;
  float a1, a2;
  float x1, x2;
  float y1, y2;
};
Biquad notch50;
//initialize the coefficients used by the iir filter
void biquadInit(Biquad* f, float b0, float b1, float b2, float a1, float a2) {
  f->b0=b0; f->b1=b1; f->b2=b2;
  f->a1=a1; f->a2=a2;
  f->x1=f->x2=f->y1=f->y2=0.0f;
}
//implement the iir filter
float biquadProcess(Biquad* f, float x) {
  float y = f->b0*x + f->b1*f->x1 + f->b2*f->x2 - f->a1*f->y1 - f->a2*f->y2;
  f->x2=f->x1; f->x1=x;
  f->y2=f->y1; f->y1=y;
  return y;
}

//for sending the values with ble
float ble_buffer[5];
int buffer_index = 0;

//variables used in the BPM detection
float dc_offset = 0.0f;
float last_notched = 0.0f;
float last_signal_val = 0.0f;
unsigned long last_peak_time = 0;
float bpm = 0.0f;

void setup() {
  Serial.begin(115200);
  delay(500);
  //start the I2C protocol
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);
  //Initialize the display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error: Couldn't find the display!"));
    while(true);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println("ECG Start..");
  display.display();
  //Initialize the ADC
  if (!ads.begin(0x48)) {
    Serial.println("Error: Couldn't find ADS1115!");
    while (true) delay(1000);
  }
  ads.setGain(GAIN_TWO);             
  ads.setDataRate(RATE_ADS1115_860SPS);
  //Initialize the iir filter
  biquadInit(&notch50, 0.9535f, -1.2258f, 0.9535f, -1.2213f, 0.9025f);
  Serial.println("Starting systemm");
  last_sample_us = micros();  
  //Setting up BLE so it alwayes transmits (notify)
  Serial.println("Starting BLE work!");
  BLEDevice::init("EcG");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  //setting the permissions of BLE characteristic
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE | 
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
                                       
  pCharacteristic->addDescriptor(new BLE2902());
  //start advertising so other devices can discover it
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  Serial.println("Esp is advertising");
}


void loop() {
  unsigned long now_us = micros();
  unsigned long now_ms = millis();
//this way it only reads the value every 2777 μs
if (now_us - last_sample_us >= SAMPLE_PERIOD_US) {
    
    last_sample_us = now_us; 

    //reads the value from the ADC
    int16_t raw = ads.readADC_SingleEnded(0);
    
    //it offsets the value so the minimum value isn't 0
    dc_offset = dc_offset * 0.95f + (float)raw * 0.05f;
    float centered_signal = (float)raw - dc_offset;
    //removes the 50hz using the iir filter
    float notched_signal = biquadProcess(&notch50, centered_signal);
    //it smoothes the filter
    static float smoothed_signal = 0.0f;
    float alpha = 0.3f; 
    //smoothing the signal using a weight (alpha)
    smoothed_signal = (smoothed_signal * (1.0f - alpha)) + (notched_signal * alpha);
    //save the value on the array
    ble_buffer[buffer_index] = smoothed_signal;
    buffer_index++;
    //send every 5th value so the ble channel doesn't get stuffed
    if (buffer_index >= 5) {
      //convert array to bytes and send the data
      pCharacteristic->setValue((uint8_t*)ble_buffer, sizeof(ble_buffer)); 
      pCharacteristic->notify();
      buffer_index = 0;
    }

    //for detecting the BPM we set a constant threshol, based on the output of the simulator. Every time the signal is higher than this threshold, then
    //we have detected an R peak.
    float val = smoothed_signal;     
    float my_hard_threshold = 18000.0f; 
    //check if we read a value more than the threshold
    if (val > my_hard_threshold) {
      //we have a 300ms gap so the same r peak isn't read twice
      if ((now_ms - last_peak_time) > 300) {
        //find the time difference between this and the last r-peak
        float rr_interval = (float)(now_ms - last_peak_time);
        //divide 60 by this difference
        float current_bpm = 60000.0f / rr_interval;        
        if (current_bpm > 40 && current_bpm < 180) {
          if (bpm == 0.0f) {
              bpm = current_bpm;
          } else {
            //smoothing the bpm value
              bpm = (bpm * 0.7f) + (current_bpm * 0.3f); 
          }
        }
        last_peak_time = now_ms;
      }
    }
    Serial.print(val);
    Serial.print(",");
    Serial.println(my_hard_threshold);
}

  //print the values on the oled (once every second so it doesn't slow the code too much)
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