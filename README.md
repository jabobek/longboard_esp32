# ESP32 4WD Longboard Remote Control System

This repository contains the complete firmware for a distributed ESP32-based ecosystem to control a 4WD electric longboard. The system is split into two main components:
- **Controller (BOARD_ID=1)**: The main unit that drives the motors and reads board sensors.
- **Remote (BOARD_ID=2)**: The handheld remote with an OLED display for user input and telemetry.

## Core Features
- **4WD Independent Motor Control**: Manages four motors with individual PWM, braking, and PID-based traction control.
- **Real-time Telemetry**: Uses ESP-NOW for low-latency communication of speed, battery status, and temperatures.
- **Web Interface**: Remote board hosts a Wi-Fi Access Point for a web-based dashboard, settings panel and update firmware interface. Controller board hosts firmware update only.
- **OTA Updates**: Wirelessly update the firmware via the web interface.
- **Advanced Safety**: Includes a advanced failsafe system for connection lost and low-battery cut-off to protect hardware.
- **On-the-fly Configuration**: Adjust drive profiles, sensor calibrations, and safety features in real-time.
- **Pairing**: Any remote can be paired to any controller. Multiple boards in same area support at same time.

## Getting Started

### 1. Uploading Firmware
Before you can use the system, you need to compile and upload the firmware to both the Remote and the Controller boards. Same source code compiled to every board based on the configuration.

The project upload and monitoring is configured to use specific COM ports. To change them, edit the **`platformio.ini`** file:

- **For the Remote**: Change the `upload_port` under the `[env:boardRemote]` section.
- **For the Controller**: Change the `upload_port` under the `[env:boardController]` section.

**Example for the Remote:**
```ini
[env:boardRemote]
upload_port = COM6
```
Then, use PlatformIO to upload the firmware to each board environment.

### 2. Wi-Fi Connection
Once the firmware is running on the Remote, it will create a Wi-Fi network.

- **Network Name (SSID)**: `LongboardRemote`
- **Password**: `12345678`

After connecting, open `192.168.4.1` in your web browser to access the dashboard and settings.

### 3. Pairing
After openning the web app, navigate to Settings via button or `192.168.4.1/settings` and press Pairing.
Remote will connect to first non-connected controller nearby.

Remote will connect to last known Controller next startup.


## Hardware & Schematics
A complete circuit diagram for both units is available in the **/resources** folder of this repository.

---

## Pinout Reference

### 1. Controller (ESC) - BOARD 1

#### Motor & Sensor Groups
*   **M1 (Front-Left)**: PWM Pin 32 (PWM Output), Brake Pin 27 (Digital Output), Hall Pin 15 (Digital Input)
*   **M2 (Front-Right)**: PWM Pin 33 (PWM Output), Brake Pin 14 (Digital Output), Hall Pin 4 (Digital Input)
*   **M3 (Rear-Left)**: PWM Pin 25 (PWM Output), Brake Pin 12 (Digital Output), Hall Pin 16 (Digital Input)
*   **M4 (Rear-Right)**: PWM Pin 26 (PWM Output), Brake Pin 13 (Digital Output), Hall Pin 17 (Digital Input)

#### General I/O & Sensors
*   **Direction Output**: Pin 23 & 5 (Digital Output)
*   **Light Output**: Pin 18 (Digital Output)
*   **Battery Voltage, use resistor divider**: Pin 35 (Analog Input)
*   **Current Sensor**: I2C Interface (INA219 at 0x40)
*   **Temperature (NTC), use resistor divider**: Pin 34 - Battery (Analog Input), Pin 39 - ESC (Analog Input)
*   **Charger Detection, use resistor divider**: Pin 19 (Digital Input)
*   **I2C Bus**: SDA Pin 21, SCL Pin 22

### 2. Remote (Handheld) - BOARD 2

#### User Inputs
*   **Analog Bpeed**: Pin 34 (Analog Input)
*   **Analog Brake**: Pin 32 (Analog Input)
*   **Brake Button**: Pin 26 (Digital Input, Pull-up, Wake-up/Power on button for deep sleep)
*   **Direction Button**: Pin 27 (Digital Input, Pull-up)
*   **Light - short press/Profile switch - long press Button**: Pin 25 (Digital Input, Pull-up)

#### Display & Sensors
*   **OLED Display 128x64px**: I2C Interface (SSD1306, SDA: 21, SCL: 22)
*   **Remote Battery Voltage, use resistor divider**: Pin 35 (Analog Input)
*   **Charger Detection, use resistor divider**: Pin 33 (Digital Input)
