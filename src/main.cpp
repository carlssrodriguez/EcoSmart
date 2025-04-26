#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <math.h>

// -------------------- PIN DEFINITIONS --------------------
#define LIGHT_SENSOR_PIN 33   // LDR connected to GPIO 33
#define LED_PIN 25            // LED connected to GPIO 25
#define LSM6DSO_ADDR 0x6B     // I2C address of accelerometer

// -------------------- THRESHOLDS --------------------
const int lightThreshold = 2500;     // Light intensity threshold
const float motionThreshold = 0.05;   // Motion detection threshold (deviation from 1g)

// -------------------- DISPLAY --------------------
TFT_eSPI tft = TFT_eSPI();

// -------------------- ACCELEROMETER READING --------------------
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
  Wire.begin(21, 22); // SDA=21, SCL=22 for TTGO

  // Initialize accelerometer (416Hz, ±2g)
  Wire.beginTransmission(LSM6DSO_ADDR);
  Wire.write(0x10); 
  Wire.write(0x60); 
  Wire.endTransmission();

  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize TFT display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(20, 50);
  tft.print("System Ready...");
  delay(1000);
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

  // Print sensor data to Serial Monitor
  Serial.printf("Light Level: %d | Motion: %s\n", lightLevel, motionDetected ? "YES" : "NO");
  Serial.printf("Accel [g]: X=%.3f | Y=%.3f | Z=%.3f | Mag=%.3f | Dev=%.3f\n", ax, ay, az, magnitude, deviation);

  // Control LED based on light and motion detection
  digitalWrite(LED_PIN, deviceOn ? HIGH : LOW);
  Serial.println(deviceOn ? "Devices ON - Active or Dark Room" : "Devices OFF - Energy Saved");

  // Update TFT display
  tft.fillScreen(TFT_BLACK);

  tft.setCursor(10, 20);
  tft.printf("Light: %d", lightLevel);

  tft.setCursor(10, 50);
  tft.printf("Motion: %s", motionDetected ? "YES" : "NO");

  tft.setCursor(10, 80);
  tft.printf("Z: %.2f g", az);

  tft.setCursor(10, 110);
  tft.print(deviceOn ? "Devices ON" : "Devices OFF");

  delay(2000);
}
