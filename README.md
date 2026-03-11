# ESP32 Longboard Remote Control System

This repository contains the firmware for an ESP32-based Longboard Remote Control system, designed to control an electric longboard (Controller) and provide user interface and input (Remote).

## Pinout Information

This section details the pin assignments for both the Controller (ESC) and Remote boards.

### 1. Controller (ESC) - BOARD 1

#### Motor & Sensor Groups

##### Front-Left (FL) / M1
*   **PWM Output**: Pin 32
*   **Brake Output**: Pin 27
*   **Hall Sensor Input**: Pin 15

##### Front-Right (FR) / M2
*   **PWM Output**: Pin 33
*   **Brake Output**: Pin 14
*   **Hall Sensor Input**: Pin 4

##### Rear-Left (RL) / M3
*   **PWM Output**: Pin 25
*   **Brake Output**: Pin 12
*   **Hall Sensor Input**: Pin 16

##### Rear-Right (RR) / M4
*   **PWM Output**: Pin 26
*   **Brake Output**: Pin 13
*   **Hall Sensor Input**: Pin 17

#### General I/O

*   **Direction Output**: Pin 23
*   **Light Output**: Pin 22

#### Analog Sensors

*   **Battery Voltage (ESC)**: Pin 35
*   **Current Sensor**: Pin 36
*   **NTC Battery Temperature**: Pin 34
*   **NTC ESC Temperature**: Pin 39

### 2. Remote (Controller) - BOARD 2

#### User Inputs

*   **Joystick X-axis (Throttle)**: Pin 34 (Analog Input)
*   **Analog Brake**: Pin 32 (Analog Input)
*   **Button Brake**: Pin 26 (Digital Input, Pull-up)
*   **Direction Button**: Pin 27 (Digital Input, Pull-up)
*   **Light Button**: Pin 25 (Digital Input, Pull-up)

#### Sensors

*   **Remote Battery Voltage**: Pin 35 (Analog Input)

#### Display (LCD)

*   **RS (Register Select)**: Pin 19
*   **EN (Enable)**: Pin 23
*   **D4 (Data Line 4)**: Pin 18
*   **D5 (Data Line 5)**: Pin 4
*   **D6 (Data Line 6)**: Pin 17
*   **D7 (Data Line 7)**: Pin 16
