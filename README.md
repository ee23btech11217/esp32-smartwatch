
ESP32 Smartwatch
================

A smartwatch project built using the ESP32 and ESP-IDF. It features an LVGL-powered display interface, 
MAX30100 heart rate sensor integration, Wi-Fi capabilities, and a real-time OS architecture using FreeRTOS.

Features
--------
- Real-time application using FreeRTOS
- GUI powered by LVGL
- Heart rate monitoring using MAX30100
- Wi-Fi configuration and event handling
- Support for I2C, SPI, and GPIO peripherals

Project Structure
-----------------
main/
├── main.c              # Application entry point
├── lvglPort.c/h        # LVGL platform driver integration
├── max30100.c/h        # MAX30100 sensor drivers
├── wifiManager.c/h     # Wi-Fi management code
├── ui.c                # UI code and LVGL bindings
├── timeSync.c          # Time synchronization functions
├── notify.c            # Notifications and alerts
├── registers.h         # Device registers

Requirements
------------
- Use Linux 
- ESP-IDF v5.x: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/
- ESP32 Development Board (e.g., ESP32-WROOM)
- ILI9341 display
- MPU9250 sensor
- MAX30100 sensor

Getting Started
---------------

1. Set Up ESP-IDF
   Follow the ESP-IDF setup guide: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html

   Example for Linux/macOS:
   $ git clone --recursive https://github.com/espressif/esp-idf.git
   $ cd esp-idf
   $ ./install.sh
   $ ./export.sh

2. Clone this Repository
   $ git clone https://github.com/ee23btech11217/esp32-smartwatch.git
   $ cd esp32-smartwatch
   $ git checkout mnepraj

3. Configure the Project
   $ cd ~/esp/esp-idf
   $ ./export.sh
   $ cd ../../esp32-smartwatch
   $ idf.py menuconfig

4. Build the Project
   $ idf.py build

5. Flash to ESP32
   Connect the board via USB, then:
   $ idf.py -p /dev/ttyUSB0 flash
   (Replace /dev/ttyUSB0 with your port, e.g., COM3 on Windows)

6. Monitor Serial Output
   $ idf.py -p /dev/ttyUSB0 monitor
   (Exit with Ctrl + ])

Troubleshooting
---------------
- Device not found: Check port name using ls /dev/tty* or Device Manager
- Flashing errors: Try idf.py erase_flash before re-flashing
- Build errors: Ensure all required components are in components/ or installed via idf.py

License
-------
This project is licensed under the MIT License. See LICENSE file for details.

Acknowledgments
---------------
- ESP-IDF: https://github.com/espressif/esp-idf
- LVGL: https://github.com/lvgl/lvgl