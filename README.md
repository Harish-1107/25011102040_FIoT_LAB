# Fundamentals of IoT (FIoT) Laboratory Submission

This repository contains Harish S.'s Fundamentals of IoT laboratory work for CAT1-CAT15, organized by microcontroller platform. The documentation below reflects the folders and files currently present in the repository; source code, project files, generated binaries, and circuit images are not modified by this documentation audit.

## 1. Student and Course Information

| Field | Detail |
|---|---|
| **Student Name** | Harish S. |
| **Registration Number** | 25011102040 |
| **Class** | IOT-A |
| **Course** | 25CS2741 – Fundamentals of IoT + Lab |
| **Department** | Department of Computer Science & Engineering |
| **Institution** | Shiv Nadar University Chennai |
| **Hardware Platforms** | 8051 Microcontroller (AT89S52-based projects) and STM32 (Nucleo-F401RE / Blue Pill F103) |

---

## 2. Repository Structure

The repository is organized into two platform directories. CAT1-CAT7 use the 8051 platform; CAT8-CAT15 use STM32 boards. Several CATs contain task-specific subdirectories, and some Keil `Objects` directories contain generated build artifacts.

```text
25011102040_FIoT_LAB/
│
├── .gitignore
├── README.md
│
├── 8051/                               # 8051 Microcontroller Platform (CAT 1 - CAT 7)
│   ├── README.md
│   ├── CAT1/                           # 8051 introductory experiment
│   ├── CAT2/                           # toolchain setup and LED blink
│   ├── CAT3/                           # timer-independent LED patterns and switch input
│   ├── CAT4/                           # keypad, seven-segment display, and LED control
│   ├── CAT5/                           # timer and interrupt exercises
│   ├── CAT6/                           # UART transmission and echo
│   └── CAT7/                           # LCD and keypad exercises
│
└── STM32/                              # STM32 ARM Cortex-M Platform (CAT 8 - CAT 15)
    ├── README.md
    ├── CAT8/                           # GPIO input/output, debouncing, and press classification
    ├── CAT9/                           # UART communication and command parsing
    ├── CAT10/                          # ESP8266 AT commands and HTTP server tasks
    ├── CAT11/                          # STM32–ESP8266 HTTP server and Node-RED client tasks
    ├── CAT12/                          # BME280 I2C, Bosch API, and HTTP telemetry tasks
    ├── CAT13/                          # BME280, ESP8266, HTTP, and Node-RED analytics
    ├── CAT14/                          # BME280 telemetry over MQTT
    └── CAT15/                          # MQTT telemetry and OpenWeather comparison
```

### Direct indexes

- [8051 experiments](8051/README.md)
- [STM32 experiments](STM32/README.md)

---

## 3. Platform Overview

### 3.1 8051 microcontroller platform

The 8051 work uses AT89S52-based projects and Keil C51 project files. Pin assignments and timing details are experiment-specific; each experiment README is the authoritative local reference.

### 3.2 STM32 ARM Cortex-M platform

The STM32 work uses STM32F103C8T6 Blue Pill and STM32F401RB/RE Nucleo projects generated with STM32CubeMX and built with Keil MDK-ARM. UART, I2C, ESP8266, BME280, MQTT, HTTP, and Node-RED details are documented in the relevant task README.

## 4. Documentation and verification scope

- Relative links are kept within the existing directory hierarchy.
- Circuit images and firmware are referenced for verification only and are not changed by this audit.
- Claims that depend on hardware measurements or external services should be treated as observed results for the stated setup, not universal performance guarantees.
