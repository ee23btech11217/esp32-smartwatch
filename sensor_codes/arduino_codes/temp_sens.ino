#include <Wire.h>
#include "MAX30105.h" // Include the MAX30105 library

MAX30105 particleSensor;

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate
  Serial.println("Initializing...");

  // Initialize the MAX30105 sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { // Use default I2C port with 400kHz speed
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1); // Stay here forever if sensor initialization fails
  }

  // Configure the sensor
  particleSensor.setup(); // Default sensor setup
  particleSensor.enableDIETEMPRDY(); // Enable the temperature ready interrupt
}

void loop() {
  // Read temperature in Celsius
  float temperatureC = particleSensor.readTemperature();

  // Read temperature in Fahrenheit
  float temperatureF = particleSensor.readTemperatureF();

  // Print the temperature values
  Serial.print("Temperature (C): ");
  Serial.print(temperatureC, 4); // Print Celsius with 4 decimal places
  Serial.print(" | Temperature (F): ");
  Serial.print(temperatureF, 4); // Print Fahrenheit with 4 decimal places
  Serial.println();

  delay(1000); // Wait for 1 second before reading again
}