#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>

// Pin Definitions for Smartwatch
#define ONE_WIRE_BUS 4  // Temperature sensor data pin
#define BATTERY_PIN 35  // Battery voltage reading pin

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Sensor and Display Objects
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Temperature Tracking Variables
struct TemperatureData {
  float currentTemp = 0.0;
  float maxTemp = -55.0;  // Minimum possible temperature
  float minTemp = 125.0;  // Maximum possible temperature
  unsigned long lastUpdateTime = 0;
} tempData;

// Battery Monitoring
struct BatteryStatus {
  float voltage = 0.0;
  int percentage = 0;
} battery;

// Smartwatch Modes
enum WatchMode {
  TEMPERATURE_MODE,
  BATTERY_MODE,
  HEALTH_MODE
};
WatchMode currentMode = TEMPERATURE_MODE;

// Button Pins for Mode Switching
const int BUTTON_PIN = 2;  // Mode change button
volatile bool buttonPressed = false;

void IRAM_ATTR handleButtonPress() {
  buttonPressed = true;
}

void setup() {
  // Initialize Serial Communication
  Serial.begin(115200);
  
  // Initialize OLED Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Display initialization failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Initialize Temperature Sensor
  tempSensor.begin();
  
  // Button Setup for Mode Switching
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);
  
  // Battery Monitoring Setup
  pinMode(BATTERY_PIN, INPUT);
}

void readTemperature() {
  tempSensor.requestTemperatures();
  float tempC = tempSensor.getTempCByIndex(0);
  
  if (tempC != DEVICE_DISCONNECTED_C) {
    tempData.currentTemp = tempC;
    
    // Update min and max temperatures
    tempData.minTemp = min(tempData.minTemp, tempC);
    tempData.maxTemp = max(tempData.maxTemp, tempC);
    
    tempData.lastUpdateTime = millis();
  }
}

void readBatteryLevel() {
  // ESP32 ADC reading (adjust based on your voltage divider)
  float rawValue = analogRead(BATTERY_PIN);
  
  // Convert to actual voltage 
  // Adjust these values based on your specific battery and voltage divider
  battery.voltage = (rawValue * 3.3 * 2) / 4096.0;
  
  // Simple battery percentage estimation
  battery.percentage = map(battery.voltage * 100, 330, 420, 0, 100);
  battery.percentage = constrain(battery.percentage, 0, 100);
}

void displayTemperatureMode() {
  display.clearDisplay();
  display.setTextSize(1);
  
  // Current Temperature
  display.setCursor(0,0);
  display.print("Temp: ");
  display.print(tempData.currentTemp, 1);
  display.println(" C");
  
  // Min/Max Temperatures
  display.print("Min: ");
  display.print(tempData.minTemp, 1);
  display.print(" C  Max: ");
  display.print(tempData.maxTemp, 1);
  display.println(" C");
  
  // Health Insights
  display.println("\nHealth Insights:");
  if (tempData.currentTemp < 35.0) {
    display.println("Low Body Temp - Risk");
  } else if (tempData.currentTemp > 37.5) {
    display.println("High Temp - Fever!");
  } else {
    display.println("Normal Temperature");
  }
  
  display.display();
}

void displayBatteryMode() {
  display.clearDisplay();
  display.setTextSize(2);
  
  // Battery Percentage
  display.setCursor(0,0);
  display.print(battery.percentage);
  display.println("%");
  
  // Voltage
  display.setTextSize(1);
  display.print("Voltage: ");
  display.print(battery.voltage, 2);
  display.println("V");
  
  // Battery Status
  display.println("\nBattery Status:");
  if (battery.percentage > 75) {
    display.println("Fully Charged");
  } else if (battery.percentage > 50) {
    display.println("Good");
  } else if (battery.percentage > 20) {
    display.println("Low - Charge Soon");
  } else {
    display.println("Critical - Charge!");
  }
  
  display.display();
}

void loop() {
  // Read temperature periodically
  static unsigned long tempReadTime = 0;
  if (millis() - tempReadTime > 5000) {
    readTemperature();
    readBatteryLevel();
    tempReadTime = millis();
  }
  
  // Handle mode switching
  if (buttonPressed) {
    // Cycle through modes
    currentMode = (WatchMode)(((int)currentMode + 1) % 3);
    buttonPressed = false;
    delay(200); // Debounce
  }
  
  // Display based on current mode
  switch (currentMode) {
    case TEMPERATURE_MODE:
      displayTemperatureMode();
      break;
    case BATTERY_MODE:
      displayBatteryMode();
      break;
    case HEALTH_MODE:
      // Placeholder for future health tracking mode
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0,0);
      display.println("Health Mode");
      display.println("Coming Soon!");
      display.display();
      break;
  }
  
  delay(100);
}