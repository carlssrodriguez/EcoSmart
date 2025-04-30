#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <TFT_eSPI.h>
#include <NimBLEDevice.h>
#include <math.h>

// -------------------- PIN DEFINITIONS --------------------
#define LIGHT_SENSOR_PIN 33
#define LED_PIN 25
#define LSM6DSO_ADDR 0x6B

// -------------------- WIFI CREDENTIALS --------------------
const char* ssid = "langas";
const char* password = "cabritas";

// -------------------- SERVER SETTINGS --------------------
const char* serverAddress = "18.144.69.140"; 
const int serverPort = 5000;

// -------------------- THRESHOLDS --------------------
const int lightThreshold = 2500;
const float motionThreshold = 0.04;

// -------------------- DISPLAY --------------------
TFT_eSPI tft = TFT_eSPI();

// -------------------- BLUETOOTH --------------------
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
NimBLECharacteristic* pCharacteristic = nullptr;

// -------------------- READ ACCELEROMETER AXIS --------------------
int16_t readAxis(uint8_t reg) {
  Wire.beginTransmission(LSM6DSO_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(LSM6DSO_ADDR, 2);

  uint8_t low = Wire.read();
  uint8_t high = Wire.read();
  return (int16_t)(high << 8 | low);
}

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);

  // Initialize I2C
  Wire.begin(21, 22);

  // Initialize Accelerometer
  Wire.beginTransmission(LSM6DSO_ADDR);
  Wire.write(0x10);
  Wire.write(0x60); 
  Wire.endTransmission();

  // Initialize Pins
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize Display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 50);
  tft.println("System Ready!");

  // Initialize Bluetooth BLE
  NimBLEDevice::init("EcoSmart");
  NimBLEServer* pServer = NimBLEDevice::createServer();
  NimBLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  pCharacteristic->setValue("System Ready");
  pService->start();
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("Bluetooth started as EcoSmart");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 50);
  tft.println("Connecting WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 50);
  tft.println("WiFi Connected!");
}

// -------------------- LOOP --------------------
void loop() {
  int lightLevel = analogRead(LIGHT_SENSOR_PIN);

  int16_t rawX = readAxis(0x28);
  int16_t rawY = readAxis(0x2A);
  int16_t rawZ = readAxis(0x2C);

  float ax = rawX * 0.061 / 1000.0;
  float ay = rawY * 0.061 / 1000.0;
  float az = rawZ * 0.061 / 1000.0;

  float magnitude = sqrt(ax * ax + ay * ay + az * az);
  float deviation = abs(magnitude - 1.0);
  bool motionDetected = deviation > motionThreshold;

  bool isDark = lightLevel < lightThreshold;
  bool deviceOn = motionDetected || isDark;

  // Update LED
  digitalWrite(LED_PIN, deviceOn ? HIGH : LOW);

  // Update Display
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 10);
  tft.printf("Light: %d", lightLevel);
  tft.setCursor(0, 30);
  tft.printf("Motion: %s", motionDetected ? "YES" : "NO");
  tft.setCursor(0, 50);
  tft.printf("Z: %.2f g", az);
  tft.setCursor(0, 70);
  tft.printf("%s", deviceOn ? "Devices ON" : "Devices OFF");

  // Send data to server
  WiFiClient client;
  HttpClient http(client);

  String path = "/?var=Light:" + String(lightLevel);
  path += "-Motion:" + String(motionDetected ? "YES" : "NO");

  Serial.println("Sending data to server...");
  int err = http.get(serverAddress, serverPort, path.c_str());

  if (err == 0) {
    Serial.println("Request sent.");
    int statusCode = http.responseStatusCode();
    Serial.printf("Status code: %d\n", statusCode);

    http.skipResponseHeaders();
    while (http.available()) {
      char c = http.read();
      Serial.print(c);
    }
  } else {
    Serial.printf("Connection error: %d\n", err);
  }

  http.stop();

  // Send data via Bluetooth (UTF-8 formatted)
  std::string btData = "Light: " + std::to_string(lightLevel) + " Motion: " + (motionDetected ? "YES" : "NO");
  pCharacteristic->setValue(btData);
  pCharacteristic->notify();

  delay(1000);
}
