#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// MPU6050 configuration
Adafruit_MPU6050 mpu;

// Advanced Step Detection Variables
const int WINDOW_SIZE = 10;
float accelerationWindow[WINDOW_SIZE];
int windowIndex = 0;
float peakThreshold = 1.2;  // Adjusted peak detection threshold
float valleyThreshold = 0.8; // Adjusted valley detection threshold
int stepCount = 0;
unsigned long lastStepTime = 0;
const int STEP_DEBOUNCE_TIME = 250; // Minimum time between steps (ms)

// User Profile for Accurate Calorie Calculation
struct UserProfile {
  float weight;     // in kg
  float height;     // in cm
  int age;          // in years
  char gender;      // 'M' or 'F'
  float walkingSpeed; // m/s
} user;

// MET (Metabolic Equivalent of Task) Values
const float MET_WALKING[] = {
  2.0,  // Slow walking (< 2 mph)
  2.8,  // Moderate walking (2-3 mph)
  3.5,  // Brisk walking (3-4 mph)
  4.3,  // Very brisk walking (4-5 mph)
  5.0   // Fast walking (> 5 mph)
};

// Advanced Sensor Data
struct SensorData {
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
  float temperature;
} sensorData;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 connection failed");
    while (1) delay(10);
  }

  // Advanced MPU6050 Configuration
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_42_HZ);

  Serial.println("MPU6050 initialized successfully");

  // Set User Profile (example values - should be customized)
  user = {
    .weight = 70.0,     // kg
    .height = 170.0,    // cm
    .age = 30,          // years
    .gender = 'M',      // Male
    .walkingSpeed = 1.4 // m/s (moderate walking)
  };
}

// Advanced Step Detection Algorithm
bool detectStep() {
  // Calculate total acceleration magnitude
  float accelMagnitude = sqrt(
    pow(sensorData.accelX, 2) + 
    pow(sensorData.accelY, 2) + 
    pow(sensorData.accelZ, 2)
  );

  // Circular buffer for windowed analysis
  accelerationWindow[windowIndex] = accelMagnitude;
  windowIndex = (windowIndex + 1) % WINDOW_SIZE;

  // Peak and valley detection
  bool isPeak = isLocalPeak(accelerationWindow, WINDOW_SIZE);
  bool isValley = isLocalValley(accelerationWindow, WINDOW_SIZE);

  // Step detection with temporal and magnitude constraints
  unsigned long currentTime = millis();
  if ((isPeak || isValley) && 
      (currentTime - lastStepTime > STEP_DEBOUNCE_TIME)) {
    lastStepTime = currentTime;
    return true;
  }

  return false;
}

// Local peak detection
bool isLocalPeak(float* data, int size) {
  int midIndex = size / 2;
  float midValue = data[midIndex];

  for (int i = 0; i < size; i++) {
    if (i != midIndex && midValue < data[i]) {
      return false;
    }
  }

  return midValue > peakThreshold;
}

// Local valley detection
bool isLocalValley(float* data, int size) {
  int midIndex = size / 2;
  float midValue = data[midIndex];

  for (int i = 0; i < size; i++) {
    if (i != midIndex && midValue > data[i]) {
      return false;
    }
  }

  return midValue < valleyThreshold;
}

// Advanced Calorie Calculation
float calculateCaloriesBurned() {
  // Determine walking intensity based on speed
  int walkingIntensityIndex = 0;
  if (user.walkingSpeed < 0.9) walkingIntensityIndex = 0;
  else if (user.walkingSpeed < 1.3) walkingIntensityIndex = 1;
  else if (user.walkingSpeed < 1.8) walkingIntensityIndex = 2;
  else if (user.walkingSpeed < 2.2) walkingIntensityIndex = 3;
  else walkingIntensityIndex = 4;

  // MET-based calorie calculation
  float met = MET_WALKING[walkingIntensityIndex];

  // Calories burned per minute
  float caloriesPerMinute = 0.0175 * met * user.weight;

  // Estimate duration based on step count and average step frequency
  float durationInMinutes = stepCount / 120.0;

  return caloriesPerMinute * durationInMinutes;
}

// Read comprehensive sensor data
void readSensorData() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  sensorData = {
    .accelX = a.acceleration.x,
    .accelY = a.acceleration.y,
    .accelZ = a.acceleration.z,
    .gyroX = g.gyro.x,
    .gyroY = g.gyro.y,
    .gyroZ = g.gyro.z,
    .temperature = temp.temperature
  };
}

void loop() {
  // Read comprehensive sensor data
  readSensorData();

  // Detect steps
  if (detectStep()) {
    stepCount++;
  }

  // Calculate calories
  float caloriesBurned = calculateCaloriesBurned();

  // Print detailed sensor data
  Serial.println("--- Advanced MPU6050 Sensor Data ---");
  Serial.print("Acceleration (m/s²): X="); Serial.print(sensorData.accelX);
  Serial.print(" Y="); Serial.print(sensorData.accelY);
  Serial.print(" Z="); Serial.println(sensorData.accelZ);

  Serial.print("Gyroscope (rad/s): X="); Serial.print(sensorData.gyroX);
  Serial.print(" Y="); Serial.print(sensorData.gyroY);
  Serial.print(" Z="); Serial.println(sensorData.gyroZ);

  

  // Print step and calorie information
  Serial.print("Step Count: "); Serial.println(stepCount);
  Serial.print("Calories Burned: "); 
  Serial.print(caloriesBurned, 2);
  Serial.println(" kcal");

  delay(500);
}