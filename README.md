EcoSmart: Energy-Aware IoT System with Cloud & Bluetooth Integration

This project implements a smart energy monitoring system using an ESP32-based TTGO LoRa32 board equipped with an LDR light sensor and built-in accelerometer (LSM6DSO). The device monitors ambient light and motion to intelligently control a connected LED, simulating energy usage based on room conditions. It displays real-time system status on a built-in TFT screen and communicates both with the cloud (via Wi-Fi) and locally (via Bluetooth).

Sensor data is transmitted to a Flask-based web server hosted on an AWS EC2 instance using HTTP GET requests. Each reading includes light intensity and motion detection status. The system is capable of operating on mobile hotspots and logs the data to enable potential analysis for optimization. Bluetooth communication is also implemented, allowing nearby devices to receive system status in UTF-8 format.

The device connects over I²C (SDA: GPIO21, SCL: GPIO22) to the onboard LSM6DSO accelerometer, uses GPIO33 for the light sensor, and GPIO25 to control the LED. TFT display output offers immediate user feedback such as "Energy Saved" when conditions are idle and "Energy Usage Detected" when activity is present.

This project integrates embedded sensing, Wi-Fi cloud connectivity, Bluetooth communication, and data display, providing a practical demonstration of a scalable and interactive IoT energy management solution.

Demo: https://youtu.be/Cvn3iM5ZI_4?si=mPoY8mg6U5ub4gkv
