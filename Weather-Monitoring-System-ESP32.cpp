#define BLYNK_TEMPLATE_ID "BLYNK_TEMPLATE_ID" //Enter Template ID
#define BLYNK_TEMPLATE_NAME "TEMPLATE_NAME" //Enter Template name
#define BLYNK_AUTH_TOKEN "Auth_Token" //Enter authentication token

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define ONE_WIRE_BUS 2
#define RELAY_PIN 17             // Pin connected to the relay
#define MOISTURE_THRESHOLD 50    // Set the moisture threshold as per needed(percentage)

int _moisture, sensor_analog;
bool manualMode = false; // Tracks if manual mode is active
bool watering = false; // Tracks if watering is happening
float rainfallChance = 0; // Store predicted rainfall chance
int cloudCover = 0; // Store cloud cover percentage

LiquidCrystal_I2C lcd(0x27, 20, 4);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
const int sensor_pin = 35;  // Soil moisture sensor pin

// WiFi Credentials
char ssid[] = "Wifi_name";     // Your WiFi SSID
char pass[] = "password";  // Your WiFi password

// Blynk Timer
BlynkTimer timer;

// OpenWeatherMap API
const char* apiKey = "api_key"; //Enter your openweatherapp API key
const char* city = "city_name"; //ENter your city
String weatherURL = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&appid=" + String(apiKey) + "&units=metric";

// Function Prototypes
void sendDataToBlynk();
void updateWateringStatus(bool status);
void fetchWeatherData();

// Blynk function to control watering manually
BLYNK_WRITE(V3) {
  int pinValue = param.asInt();
  manualMode = (pinValue == 1); // Enable/Disable manual mode

  if (manualMode) {
    digitalWrite(RELAY_PIN, LOW);  // Turn ON relay manually
    updateWateringStatus(true);
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Turn OFF relay, return to auto mode
    updateWateringStatus(false);
  }
  sendDataToBlynk(); // Update Blynk status immediately
}

// Function to send data to Blynk
void sendDataToBlynk() {
  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, sensors.getTempCByIndex(0)); // Temperature
    Blynk.virtualWrite(V1, _moisture); // Soil Moisture
    Blynk.virtualWrite(V2, watering ? 1 : 0); // Watering Status Indicator
    Blynk.virtualWrite(V4, cloudCover); // Cloud Cover
    Blynk.virtualWrite(V5, rainfallChance); // Rain Prediction
  }
}

// Function to update watering status (LCD & Blynk)
void updateWateringStatus(bool status) {
  watering = status;
  lcd.setCursor(0, 3);
  if (watering) {
    lcd.print("Watering...       ");
  } else {
    lcd.print("Not Watering      ");
  }
  sendDataToBlynk();
}

// Fetch Weather Data and Predict Rain
void fetchWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(weatherURL);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println("Weather API Response:");
      Serial.println(payload);

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      
      cloudCover = doc["clouds"]["all"];
      int weatherCode = doc["weather"][0]["id"];
      
      // Predict rain chance based on weather code & cloud cover
      if (weatherCode >= 200 && weatherCode < 600) {
        rainfallChance = 90;  // Thunderstorm, drizzle, or rain → High chance
      } else if (cloudCover > 70) {
        rainfallChance = 60;  // Heavy clouds → Moderate chance
      } else {
        rainfallChance = 10;  // Clear or light clouds → Low chance
      }

      Serial.print("Predicted Rain Chance: ");
      Serial.println(rainfallChance);
    } else {
      Serial.print("Error in HTTP request: ");
      Serial.println(httpResponseCode);
      rainfallChance = 10;  // Default to low chance if request fails
    }
    http.end();
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");

  sensors.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Start with relay off

  WiFi.begin(ssid, pass);
  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  timer.setInterval(10000L, sendDataToBlynk);
  timer.setInterval(30000L, fetchWeatherData); // Fetch weather every 30 sec
}

void loop() {
  sensor_analog = analogRead(sensor_pin);
  _moisture = (100 - ((sensor_analog / 4095.00) * 100));
  Serial.print("Moisture = ");
  Serial.print(_moisture);
  Serial.println("%");

  lcd.clear();
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  lcd.setCursor(0, 0);
  if (tempC != DEVICE_DISCONNECTED_C) {
    lcd.print("Temp: ");
    lcd.print(tempC);
    lcd.print(" C");
  } else {
    lcd.print("Error: No Data");
  }

  lcd.setCursor(0, 1);
  lcd.print("Moisture: ");
  lcd.print(_moisture);
  lcd.print("%");

  lcd.setCursor(0, 2);
  lcd.print("Rain: ");
  lcd.print(rainfallChance);
  lcd.print("%");

  // Relay Control Logic
  if (manualMode) {
    digitalWrite(RELAY_PIN, LOW);
    updateWateringStatus(true);
  } else if (_moisture < MOISTURE_THRESHOLD) {
    digitalWrite(RELAY_PIN, LOW);
    updateWateringStatus(true);
  } else {
    digitalWrite(RELAY_PIN, HIGH);
    updateWateringStatus(false);
  }

  Blynk.run();
  timer.run();
  delay(5000);
}

