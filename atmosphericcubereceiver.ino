#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_NeoPixel.h>
#include "esp_wifi.h"

#define LED_PIN 3
#define LED_COUNT 24
Adafruit_NeoPixel ring(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Data Struct with serial for debugging
typedef struct {
  float temp;
  float hum;
  float baropressure;
  float airquality;
} infopackage;

// Default fallback values if no data yet
infopackage incomingData = { 22.0, 75.0, 1013.0, 50.0 };

// Breathing animation vars
int breathBrightness = 0;
int breathDirection = 1;
unsigned long lastBreathUpdate = 0;

// Pulse control
int pulseDelay = 30;  // updated based on pressure

// ESP-NOW Callback with serial for debugging
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(infopackage)) {
    memcpy(&incomingData, data, sizeof(infopackage));
    Serial.println("Data received!");

    // Update pulse speed based on pressure
    pulseDelay = map((int)incomingData.baropressure, 1015, 1000, 60, 10);
    pulseDelay = constrain(pulseDelay, 10, 100);  // safe bounds
    
    // Serial to see numerical values
    Serial.printf("Temp: %.2f °C | Hum: %.2f%% | Pressure: %.2f hPa | Air Quality: %.2f\n",
      incomingData.temp, incomingData.hum, incomingData.baropressure, incomingData.airquality);

  } else {
    Serial.println("Received invalid data length");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  // Serial for ESP-NOW debugging
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true);
  }
  esp_now_register_recv_cb(onReceive);

  // LED initialization
  ring.begin();
  ring.setBrightness(100);
  ring.show();
}

void loop() {
  unsigned long now = millis();

  // Timer to keep track of intervals and perform brightness adjustment
  if (now - lastBreathUpdate >= pulseDelay) {
    lastBreathUpdate = now;

    // Update breathing brightness
    breathBrightness += breathDirection * 5;
    if (breathBrightness >= 255) {
      breathBrightness = 255;
      breathDirection = -1;
    } else if (breathBrightness <= 20) {
      breathBrightness = 20;
      breathDirection = 1;
    }

    // Map humidity to brightness modulation
    int mappedBrightness = map(constrain((int)incomingData.hum, 20, 90), 20, 90, 20, breathBrightness);
    ring.setBrightness(mappedBrightness);

    // Determine color from temperature
    uint32_t color;
    if (incomingData.temp < 17)
      color = ring.Color(0, 0, 255);  // Blue for cold
    else if (incomingData.temp < 30)
      color = ring.Color(0, 255, 0);  // Green for mild
    else
      color = ring.Color(255, 0, 0);  // Red for hot

    // Set all pixels to the same colorcolor
    for (int i = 0; i < LED_COUNT; i++)
      ring.setPixelColor(i, color);

    // Add flicker effect if air quality is poor
    if (incomingData.airquality < 50.0) {
      for (int i = 0; i < LED_COUNT; i++) {
        if (random(0, 10) < 3) {  // Random number generator for 30% flicker chance per LED
          ring.setPixelColor(i, 0);
        }
      }
    }

    ring.show(); // Update the LED ring
  }
}
