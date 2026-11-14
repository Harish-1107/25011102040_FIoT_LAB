# STM32 Platform Experiments

## 1. Platform Details

- **Microcontroller**: STM32F401RB / STM32F401RE (ARM® Cortex®-M4 with FPU @ up to 84 MHz)
- **Development Board**: STM32 Nucleo-64 (NUCLEO-F401RE)
- **Firmware Development Tools**: STM32CubeMX (v6.18.0) + Keil MDK-ARM (uVision v5.32+)
- **Software Drivers**: STM32F4xx HAL Driver + CMSIS Core + Bosch Sensortec BME280 Sensor API v3.5.1
- **Clock Configuration**: 16 MHz High-Speed Internal (HSI) RC Oscillator

---

## 2. Experiments Index

| Experiment | Task ID | Description | Status |
|---|---|---|---|
| **CAT 12** | Consolidated | BME280 Sensor Interfacing Using I2C and Bosch Sensor API + ESP8266 Wi-Fi HTTP Telemetry | **COMPLETED** |
| **CAT 12** | TASK 12A | BME280 Sensor Hardware Bring-up, I2C Interface & Chip ID Verification (0x60) | **COMPLETED** |
| **CAT 12** | TASK 12B | Environmental Data Acquisition, Calibration Trimming & Bosch Compensation Algorithms | **COMPLETED** |
| **CAT 12** | TASK 12C | ESP8266 AT Command TCP Socket Bridge & HTTP POST Dynamic JSON Telemetry to Node-RED | **COMPLETED** |

---

## 3. Peripheral Hardware Allocation

```text
               +--------------------------------------------+
               |           STM32F401RB (LQFP64)             |
               |                                            |
BME280 SCL <-- | PB6 (I2C1_SCL)           PA9 (USART1_TX)   | --> ESP8266 RX
BME280 SDA <-> | PB7 (I2C1_SDA)           PA10 (USART1_RX)  | <-- ESP8266 TX
               |                                            |
               |                          PA2 (USART2_TX)   | --> ST-LINK VCP RX (PC)
               |                          PA3 (USART2_RX)   | <-- ST-LINK VCP TX (PC)
               |                                            |
User LED   <-- | PA5 (GPIO Output)        PC13 (GPIO Input) | <-- User Button
               +--------------------------------------------+
```

Refer to [CAT12/README.md](CAT12/README.md) for detailed technical schematics, register walkthroughs, calibration formulas, and validation traces.
