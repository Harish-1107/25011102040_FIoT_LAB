# Fundamentals of IoT (FIoT) Laboratory Submission

## 1. Student and Course Information

| Field | Detail |
|---|---|
| **Student Name** | Harish S. |
| **Registration Number** | 25011102040 |
| **Class** | IOT-A |
| **Course** | 25CS2741 – Fundamentals of IoT + Lab |
| **Department** | Department of Computer Science & Engineering |
| **Institution** | Shiv Nadar University Chennai |
| **Hardware Platforms** | 8051 Microcontroller (AT89C51/52) & STM32 (Nucleo-F401RE / Blue Pill F103) |

---

## 2. Repository Structure

This repository is organized into distinct hardware platform directories complying with official coursework and automated evaluation specifications:

```text
25011102040_FIoT_LAB/
│
├── .gitignore                          # Prescribed ignore rules for build objects
├── README.md                           # Master repository documentation (this file)
│
├── 8051/                               # 8051 Microcontroller Platform (CAT 1 - CAT 7)
│   ├── README.md                       # Platform overview & task mapping
│   ├── CAT1/                           # CAT 1: GPIO & Simple LED Blinking
│   ├── CAT2/                           # CAT 2: LED Patterns & Switch Interfacing
│   ├── CAT3/                           # CAT 3: 7-Segment Display Interfacing
│   ├── CAT4/                           # CAT 4: 16x2 Character LCD Interfacing
│   ├── CAT5/                           # CAT 5: Hardware Timers & Delay Generation
│   ├── CAT6/                           # CAT 6: Serial UART Communication (8051)
│   └── CAT7/                           # CAT 7: Hardware Interrupts & Relay / Sensor Interfacing
│
└── STM32/                              # STM32 ARM Cortex-M Platform (CAT 8 - CAT 15)
    ├── README.md                       # STM32 platform overview & pin mappings
    ├── CAT8/                           # CAT 8: STM32 GPIO Bring-Up, Buttons & Multi-LEDs (TASK 8A/B/C)
    ├── CAT9/                           # CAT 9: Basic UART Communication & Command Parser (TASK 9A/B)
    ├── CAT10/                          # CAT 10: ESP8266 AT Command State Machine & Embedded Web Server (TASK 10A/B/C)
    ├── CAT11/                          # CAT 11: ADC Interfacing & Analog Sensor Conditioning (TASK 11A/B)
    ├── CAT12/                          # CAT 12: BME280 I2C Bosch Sensor API & Wi-Fi HTTP Telemetry
    ├── CAT13/                          # CAT 13: Cloud IoT Telemetry via MQTT Broker (TASK 13)
    ├── CAT14/                          # CAT 14: Bi-directional IoT Actuation & Cloud Telemetry Control (TASK 14)
    └── CAT15/                          # CAT 15: Environmental Sensing, OpenWeather API Comparison & Node-RED (TASK 15)
```

---

## 3. Platform Overview

### 3.1 8051 Microcontroller Architecture
- **Processor**: 8051 Core (AT89C51 / AT89C52 / AT89S52)
- **Clock**: 11.0592 MHz / 12 MHz Crystal Oscillator
- **Toolchain**: Keil uVision 5 (C51 Compiler & A51 Macro Assembler)
- **Key Subsystems**: Port 0–3 Bit-Addressable GPIO, Timer 0 & 1 Mode 1/2, Mode 1 8-bit UART (9600 Baud), External Interrupts INT0/INT1.

### 3.2 STM32 ARM Cortex-M Architecture
- **Processors**: STM32F401RB (Nucleo-64) & STM32F103C8T6 (Blue Pill)
- **Clock**: 16 MHz HSI / 72 MHz HSE PLL
- **Toolchain**: Keil MDK-ARM (ARM Compiler 6) + STM32CubeMX
- **Peripherals Interfaced**: I2C1 (100 kHz), USART1/2 (115200 Baud), ESP8266 Wi-Fi modem, Bosch BME280 digital environmental sensor.
