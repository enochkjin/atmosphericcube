#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_now.h>
#include "esp_wifi.h"
#include <Adafruit_BME680.h>

// SPI Pins for BME680
#define BME_SCK 5
#define BME_MISO 4
#define BME_MOSI 3
#define BME_CS   1

// Sensor Object
Adafruit_BME680 bme(BME_CS);

// Data Struct
typedef struct {
  float temp;
  float hum;
  float baropressure;
  float airquality;
} infopackage;

infopackage dataToSend;

// Receiver MAC Addy
uint8_t receiverMAC[] = { 0x34, 0xB7, 0xDA, 0xF6, 0x3C, 0x44 };

void setup() {
  Serial.begin(115200);

  // ESP-NOW Setup
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true);
  }
  Serial.println("ESP-NOW initialized");

  // Peer info for debugging
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    while (true);
  }
  Serial.println("Peer added successfully");

  // Initialize BME680
  SPI.begin(BME_SCK, BME_MISO, BME_MOSI, BME_CS);
  if (!bme.begin()) {
    Serial.println("BME680 not found");
    while (true);
  }

  // Configure BME680
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320°C for 150 ms
}


void loop() {

  // Read BME680
  if (bme.performReading()) {
    dataToSend.temp = bme.temperature;               // Celsius
    dataToSend.hum = bme.humidity;                   // Hum percentage
    dataToSend.baropressure = bme.pressure / 100.0;  // Pa → hPa
    dataToSend.airquality = bme.gas_resistance / 1000.0; // Ohm → kOhm

    // Send data to receiver
    esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)&dataToSend, sizeof(dataToSend));

    if (result == ESP_OK) {
      Serial.println("Data sent successfully");
    } else {
      Serial.println("Error sending data");
    }
  } else {
    Serial.println("BME680 reading failed");
  }

  delay(1000);
}
