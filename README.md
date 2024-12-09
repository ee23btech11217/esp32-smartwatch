| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- | ----- |

# Hello World Example

Starts a FreeRTOS task to print "Hello World".

(See the README.md file in the upper level 'examples' directory for more information about examples.)

## How to use example

Follow detailed instructions provided specifically for this example.

Select the instructions depending on Espressif chip installed on your development board:

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)
- [ESP32-S2 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)


## Example folder contents

The project **hello_world** contains one source file in C language [hello_world_main.c](main/hello_world_main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both).

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── pytest_hello_world.py      Python script used for automated testing
├── main
│   ├── CMakeLists.txt
│   └── hello_world_main.c
└── README.md                  This is the file you are currently reading
```

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

## Troubleshooting

* Program upload failure

    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.


# ESP32 with Arduino IDE

## Overview
This project demonstrates how to use **MAX30105 Pulse Oximeter** and **MPU6050 IMU** sensors simultaneously with an ESP32. Both sensors communicate over the **I2C protocol** and are programmed using the **Arduino IDE**. The guide includes flashing the ESP32, configuring multiple sensors, and handling I2C communication.

---

## Requirements

### Hardware
- ESP32 development board
- MAX30105 Pulse Oximeter sensor
- MPU6050 IMU sensor
- Jumper wires
- Breadboard

### Software
- Arduino IDE (latest version)
- ESP32 Board Manager installed in Arduino IDE
- Libraries:
  - `MAX30105` library
  - `MPU6050` library
  - `Wire` library (built-in)

---

## Setup Instructions

### 1. Configure Arduino IDE for ESP32
1. Open **Arduino IDE**.
2. Go to **File > Preferences**.
3. Add this URL to the *Additional Board Manager URLs*:  
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json


4. Go to **Tools > Board > Board Manager**, search for **ESP32**, and install the package.

### 2. Install Required Libraries
1. Go to **Tools > Manage Libraries**.
2. Search and install:
- `MAX30105` library by SparkFun
- `MPU6050` library by Electronic Cats
3. Ensure the `Wire` library is included (built into Arduino IDE).

### 3. Connect the Hardware
Connect the sensors to the ESP32 as follows:

| Pin        | ESP32         | MAX30105      | MPU6050      |
|------------|---------------|---------------|--------------|
| **SDA**    | GPIO21 (default) | SDA           | SDA          |
| **SCL**    | GPIO22 (default) | SCL           | SCL          |
| **VCC**    | 3.3V          | 3.3V          | 3.3V         |
| **GND**    | GND           | GND           | GND          |

### 4. Code Files
Two sensors are implemented, sharing the **SDA** and **SCL** lines. The **MAX30105** measures heart rate and SpO2, while the **MPU6050** reads acceleration and gyroscopic data.

---

## Flashing the ESP32
1. Connect the ESP32 to your computer via USB.
2. Open Arduino IDE and paste the code.
3. Select the correct board and port:
- **Tools > Board > ESP32 Dev Module**
- **Tools > Port > [Select Your ESP32 Port]**
4. Click **Upload** to flash the code onto the ESP32.

---

## Handling I2C Communication
Both sensors communicate on the I2C bus using distinct addresses:
- **MAX30105 I2C Address**: 0x57
- **MPU6050 I2C Address**: 0x68 (default) or 0x69

The **Wire** library manages communication, ensuring no conflicts by differentiating addresses.

---

## Sample Output
The Serial Monitor will display:
- MAX30105: Heart rate (BPM) and SpO2 levels.
- MPU6050: Accelerometer and gyroscope readings.

---

## Troubleshooting
1. Ensure correct wiring, particularly for SDA and SCL pins.
2. Use a logic level converter if sensors operate at 5V.
3. Verify I2C addresses with an I2C scanner if communication fails.

---

## License
This project is open-source and available under the MIT License.