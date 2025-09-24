# Weather Monitoring System ESP32

A simple ESP32-based weather and soil monitoring system integrated with Blynk app.

## Overview
- Reads soil moisture and temperature.
- Displays data on a 20x4 LCD.
- Sends data to the Blynk app for remote monitoring.
- Automatic or manual watering control via relay.

## Requirements
- ESP32 Dev Board
- Soil Moisture Sensor
- DS18B20 Temperature Sensor
- 20x4 I2C LCD
- Relay Module
- Jumper wires and breadboard

## Software
- Arduino IDE or Visual Studio with Arduino extension
- Libraries (included in code via `#include` statements):
  - Blynk
  - DallasTemperature
  - OneWire
  - LiquidCrystal_I2C
  - ArduinoJson

## Setup
1. Open `Weather-Monitoring-System.cpp` in Visual Studio.
2. Replace placeholders in the code:
   ```cpp
   #define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"
   char ssid[] = "YOUR_WIFI_SSID";
   char pass[] = "YOUR_WIFI_PASSWORD";
   const char* apiKey = "YOUR_OPENWEATHERMAP_API_KEY";
   const char* city   = "YOUR_CITY_NAME";
