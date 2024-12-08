#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// MAX30102 Sensor Object
MAX30105 particleSensor;

// Heart Rate Variables
const byte RATE_SIZE = 4; // Increase this for more averaging
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time of the last beat
float beatsPerMinute;
int beatAvg;
bool beatDetected = false;

// Advanced Heart Rate Tracking
unsigned long startTime;
const unsigned long MEASUREMENT_DURATION = 30000; // 30 seconds measurement window
int validReadings = 0;
float totalHeartRate = 0;

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  
  // Initialize OLED Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  
  // Initialize MAX30102 Sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
  
  // Configure Sensor
  particleSensor.setup(); // By default, use red LED
  particleSensor.setPulseAmplitudeRed(0x0A); // Adjust LED brightness
  particleSensor.setPulseAmplitudeGreen(0); // Turn off Green LED
  
  // Advanced Configuration
  particleSensor.enableDIETEMPRDY(); // Enable die temperature reading
  particleSensor.setLEDMode(MAX30105_MODE_MULTILED);
  
  // Initial Display
  displayWelcomeScreen();
  
  // Start timing
  startTime = millis();
}

void displayWelcomeScreen() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("Heart Rate Monitor"));
  display.println(F("Place finger on"));
  display.println(F("sensor..."));
  display.display();
}

void calculateHeartRate() {
  long irValue = particleSensor.getIR();
  
  if (checkForBeat(irValue) == true) {
    // We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();
    
    // Calculate heart rate
    beatsPerMinute = 60 / (delta / 1000.0);
    
    // Ensure reasonable heart rate range (30-220 bpm)
    if (beatsPerMinute < 30 || beatsPerMinute > 220) {
      return;
    }
    
    // Rolling average of heart rates
    rates[rateSpot++] = (byte)beatsPerMinute;
    rateSpot %= RATE_SIZE;
    
    // Calculate average
    beatAvg = 0;
    for (byte x = 0; x < RATE_SIZE; x++) {
      beatAvg += rates[x];
    }
    beatAvg /= RATE_SIZE;
    
    // Track for long-term measurement
    if (millis() - startTime < MEASUREMENT_DURATION) {
      totalHeartRate += beatsPerMinute;
      validReadings++;
    }
    
    beatDetected = true;
  }
}

void displayHeartRateInfo() {
  display.clearDisplay();
  display.setCursor(0,0);
  
  // Instantaneous Heart Rate
  display.print(F("HR: "));
  display.print(beatsPerMinute, 1);
  display.println(F(" bpm"));
  
  // Average Heart Rate
  display.print(F("Avg: "));
  display.print(beatAvg);
  display.println(F(" bpm"));
  
  // Measurement Quality Indicator
  long irValue = particleSensor.getIR();
  if (irValue < 50000) {
    display.println(F("No finger detected!"));
  } else {
    display.println(F("Signal Good"));
  }
  
  display.display();
}

void loop() {
  // Calculate heart rate
  calculateHeartRate();
  
  // Display heart rate info
  if (beatDetected) {
    displayHeartRateInfo();
    beatDetected = false;
  }
  
  // Long-term measurement and final average
  if (millis() - startTime >= MEASUREMENT_DURATION) {
    float finalAverage = totalHeartRate / validReadings;
    
    Serial.println("--- Heart Rate Measurement Complete ---");
    Serial.print("Final Average Heart Rate: ");
    Serial.print(finalAverage, 1);
    Serial.println(" bpm");
    
    // Optional: Additional health insights based on age and average HR
    interpretHeartRate(finalAverage);
    
    // Reset for next measurement
    startTime = millis();
    totalHeartRate = 0;
    validReadings = 0;
  }
  
  // Small delay to prevent overwhelming processing
  delay(10);
}

void interpretHeartRate(float avgHeartRate) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("Health Insights:"));
  
  // Basic heart rate interpretation
  if (avgHeartRate < 60) {
    display.println(F("Low HR"));
  } else if (avgHeartRate >= 60 && avgHeartRate <= 100) {
    display.println(F("Normal Resting HR"));
  } else if (avgHeartRate > 100 && avgHeartRate <= 150) {
    display.println(F("Elevated HR"));
  } else {
    display.println(F("Very High HR"));
  }
  
  display.display();
}