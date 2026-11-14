# STM32 Platform Experiments

This directory contains the STM32F103 and STM32F401 laboratory projects. Individual task folders include the relevant CubeMX configuration, Keil project, source files, circuit image, and task-specific documentation where those files are present.

## 1. Platform Details

- **Microcontroller**: STM32F401RB / STM32F401RE & STM32F103C8T6
- **Development Boards**: STM32 Nucleo-64 (NUCLEO-F401RE) & STM32 Blue Pill
- **Firmware Development Tools**: STM32CubeMX (v6.18.0) + Keil MDK-ARM (uVision v5.32+)
- **Software Drivers**: STM32F4xx / STM32F1xx HAL Drivers + CMSIS Core + Bosch Sensortec BME280 Sensor API

---

## 2. Experiments Index

| Experiment | Task ID | Description | Status |
|---|---|---|---|
| **CAT 8** | TASK 8A / 8B / 8C | STM32 GPIO input/output, debouncing, and press classification | **DOCUMENTED** |
| **CAT 9** | TASK 9A / 9B | UART communication, character echo, and command parsing | **DOCUMENTED** |
| **CAT 10** | TASK 10A / 10B / 10C | STM32-ESP8266 UART, AT-command control, and HTTP server | **DOCUMENTED** |
| **CAT 11** | TASK 11A / 11B | ESP8266 TCP/HTTP server and Node-RED HTTP client integration | **DOCUMENTED** |
| **CAT 12** | Master / 12A / 12B / 12C | BME280 Sensor Interfacing (I2C + Bosch API) and Wi-Fi HTTP telemetry | **DOCUMENTED** |
| **CAT 15** | TASK 15 | Real-time environmental sensing, OpenWeather comparison, and Node-RED analytics | **DOCUMENTED** |

## 3. Task links

- [CAT8 tasks](CAT8/TASK8A/README.md) · [CAT8B](CAT8/TASK8B/README.md) · [CAT8C](CAT8/TASK8C/README.md)
- [CAT9 tasks](CAT9/TASK9A/README.md) · [CAT9B](CAT9/TASK9B/README.md)
- [CAT10 tasks](CAT10/TASK10A/README.md) · [CAT10B](CAT10/TASK10B/README.md) · [CAT10C](CAT10/TASK10C/README.md)
- [CAT11 tasks](CAT11/TASK11A/README.md) · [CAT11B](CAT11/TASK11B/README.md)
- [CAT12 overview](CAT12/README.md) · [CAT12A](CAT12/TASK12A/README.md) · [CAT12B](CAT12/TASK12B/README.md) · [CAT12C](CAT12/TASK12C/README.md)
- [CAT13](CAT13/TASK13/README.md) · [CAT14](CAT14/TASK14/README.md) · [CAT15](CAT15/TASK15/README.md)

`CAT10/README.md` and `CAT12/README.md` provide additional overview documentation for their task groups.

