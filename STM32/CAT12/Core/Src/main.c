/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : CAT 12 - BME280 Sensor Interfacing Using I2C and Bosch
  *                   Sensor API with ESP8266 Wi-Fi HTTP Telemetry to Node-RED
  * @author         : Harish S. (25011102040)
  * @institution    : Shiv Nadar University Chennai
  * @course         : 25CS2741 - Fundamentals of IoT (FIoT) Lab
  ******************************************************************************
  * @attention
  *
  * Hardware Pin Mapping:
  *   I2C1:
  *     PB6 = I2C1_SCL
  *     PB7 = I2C1_SDA
  *   USART1 (ESP8266 / Wemos D1 Mini AT Bridge):
  *     PA9  = USART1_TX (Connects to ESP8266 RX)
  *     PA10 = USART1_RX (Connects to ESP8266 TX)
  *   USART2 (PC Serial Debug via ST-LINK VCP @ 115200 baud):
  *     PA2 = USART2_TX (Virtual COM Port RX)
  *     PA3 = USART2_RX (Virtual COM Port TX)
  *   GPIO:
  *     PA5  = User LED (LD2 Output)
  *     PC13 = User Button (B1 Input)
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bme280.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NODE_RED_SERVER_IP   "10.31.1.46"
#define NODE_RED_SERVER_PORT 1880
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
struct bme280_dev dev;
struct bme280_data comp_data;
int8_t rslt = BME280_OK;

/* Variables to store calculated sensor values */
float temperature, pressure, humidity;

/* Primary I2C address for BME280 (0x76 when SDO is grounded) */
static uint8_t dev_addr = BME280_I2C_ADDR_PRIM;

char debug_msg[128];
int debug_len;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
int8_t user_i2c_read(uint8_t reg_addr,
                     uint8_t *reg_data,
                     uint32_t len,
                     void *intf_ptr);

int8_t user_i2c_write(uint8_t reg_addr,
                      const uint8_t *reg_data,
                      uint32_t len,
                      void *intf_ptr);

void user_delay_us(uint32_t period, void *intf_ptr);

static void FormatFloat2(char *buffer,
                         uint32_t size,
                         float value);

static uint8_t SendBME280HTTP(float t,
                              float h,
                              float p);

static void Debug_Print(const char *text);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, initializes Flash interface and SysTick. */
    HAL_Init();

    /* USER CODE BEGIN Init */
    /* USER CODE END Init */

    /* Configure system clock (16 MHz HSI). */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals. */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    /* Indicate startup over USART2 debug console */
    const uint8_t test_msg[] = "\r\n========================================\r\n"
                               "  CAT 12: BME280 + ESP8266 + Node-RED   \r\n"
                               "  Student: Harish S. (25011102040)      \r\n"
                               "  STM32 UART2 DEBUG INTERFACE READY     \r\n"
                               "========================================\r\n";

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)test_msg,
                      sizeof(test_msg) - 1U,
                      1000U);

    /* USER CODE BEGIN 2 */

    /* Allow ESP8266 module sufficient time to boot and register on Wi-Fi */
    Debug_Print("Waiting 5s for ESP8266 boot...\r\n");
    HAL_Delay(5000U);

    /* Initial TCP probe to verify Wi-Fi module connectivity */
    uint8_t cmd[] = "AT+CIPSTART=\"TCP\",\"10.31.1.46\",1880\r\n";
    uint8_t rx_data;
    char response[256] = {0};

    HAL_UART_Transmit(&huart1,
                      cmd,
                      sizeof(cmd) - 1U,
                      1000U);

    uint16_t count = 0U;
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < 15000U &&
           count < sizeof(response) - 1U)
    {
        HAL_StatusTypeDef rx_status = HAL_UART_Receive(
            &huart1,
            &rx_data,
            1U,
            100U
        );

        if (rx_status == HAL_OK)
        {
            response[count++] = (char)rx_data;
            response[count] = '\0';

            if (strstr(response, "OK") != NULL ||
                strstr(response, "ERROR") != NULL ||
                strstr(response, "FAIL") != NULL)
            {
                break;
            }
        }
        else if (rx_status != HAL_TIMEOUT)
        {
            break;
        }
    }

    char header[] = "\r\nWemos Initial Probe Response:\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)header, sizeof(header) - 1U, 1000U);
    HAL_UART_Transmit(&huart2, (uint8_t *)response, count, 1000U);

    /* -------------------------------------------------------------
     * Step 1: Hook Bosch API Hardware Abstraction Function Pointers
     * ------------------------------------------------------------- */
    dev.intf_ptr = &dev_addr;
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;

    /* -------------------------------------------------------------
     * Step 2: Initialize BME280 Driver and Verify Chip ID
     * ------------------------------------------------------------- */
    rslt = bme280_init(&dev);

    debug_len = snprintf(debug_msg,
                         sizeof(debug_msg),
                         "BME280 Init Result: %d | Chip ID: 0x%02X (Expected: 0x60)\r\n",
                         rslt,
                         dev.chip_id);

    if ((debug_len > 0) && (debug_len < (int)sizeof(debug_msg)))
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)debug_msg,
                          (uint16_t)debug_len,
                          HAL_MAX_DELAY);
    }

    if (rslt != BME280_OK || dev.chip_id != BME280_CHIP_ID)
    {
        Debug_Print("ERROR: BME280 Initialization failed! System halted.\r\n");
        while (1)
        {
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            HAL_Delay(200);
        }
    }

    /* -------------------------------------------------------------
     * Step 3: Display Calibration Trimming Parameters
     * ------------------------------------------------------------- */
    char calib_msg[220];
    int calib_len = snprintf(
        calib_msg,
        sizeof(calib_msg),
        "\r\n--- BME280 PRESSURE CALIBRATION DATA ---\r\n"
        "dig_P1 = %u\r\n"
        "dig_P2 = %d\r\n"
        "dig_P3 = %d\r\n"
        "dig_P4 = %d\r\n"
        "dig_P5 = %d\r\n"
        "dig_P6 = %d\r\n"
        "dig_P7 = %d\r\n"
        "dig_P8 = %d\r\n"
        "dig_P9 = %d\r\n",
        (unsigned)dev.calib_data.dig_p1,
        (int)dev.calib_data.dig_p2,
        (int)dev.calib_data.dig_p3,
        (int)dev.calib_data.dig_p4,
        (int)dev.calib_data.dig_p5,
        (int)dev.calib_data.dig_p6,
        (int)dev.calib_data.dig_p7,
        (int)dev.calib_data.dig_p8,
        (int)dev.calib_data.dig_p9
    );

    HAL_UART_Transmit(&huart2, (uint8_t *)calib_msg, (uint16_t)calib_len, 1000U);

    /* -------------------------------------------------------------
     * Step 4: Configure Sensor Oversampling, Filter & Operating Mode
     * ------------------------------------------------------------- */
    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_1X;
    dev.settings.osr_t = BME280_OVERSAMPLING_1X;
    dev.settings.filter = BME280_FILTER_COEFF_OFF;

    int8_t settings_status = bme280_set_sensor_settings(
        BME280_SEL_OSR_PRESS |
        BME280_SEL_OSR_TEMP  |
        BME280_SEL_OSR_HUM   |
        BME280_SEL_FILTER,
        &dev.settings,
        &dev
    );

    /* Set normal power mode for continuous measurement */
    int8_t mode_status = bme280_set_sensor_mode(
        BME280_POWERMODE_NORMAL,
        &dev
    );

    /* Allow conversion cycle to complete */
    HAL_Delay(100);

    /* Verify configuration registers: 0xF2 (ctrl_hum), 0xF3 (status), 0xF4 (ctrl_meas) */
    {
        uint8_t ctrl_regs[3] = {0};
        HAL_StatusTypeDef reg_status;

        reg_status = HAL_I2C_Mem_Read(
            &hi2c1,
            (uint16_t)(dev_addr << 1),
            0xF2,
            I2C_MEMADD_SIZE_8BIT,
            ctrl_regs,
            3,
            1000
        );

        char config_msg[200];
        int config_len = snprintf(
            config_msg,
            sizeof(config_msg),
            "\r\n--- BME280 CONFIGURATION REGISTERS ---\r\n"
            "Settings Status      : %d\r\n"
            "Mode Status          : %d\r\n"
            "Register Read Status : %d\r\n"
            "CTRL_HUM  (0xF2)     : 0x%02X\r\n"
            "STATUS    (0xF3)     : 0x%02X\r\n"
            "CTRL_MEAS (0xF4)     : 0x%02X\r\n",
            (int)settings_status,
            (int)mode_status,
            (int)reg_status,
            ctrl_regs[0],
            ctrl_regs[1],
            ctrl_regs[2]
        );

        if (config_len > 0 && config_len < (int)sizeof(config_msg))
        {
            HAL_UART_Transmit(&huart2, (uint8_t *)config_msg, (uint16_t)config_len, 1000U);
        }
    }

    /* Check TCP probe result */
    uint8_t tcp_connected = (strstr(response, "CONNECT") != NULL || strstr(response, "OK") != NULL) ? 1U : 0U;

    if (tcp_connected)
    {
        Debug_Print("\r\n[TCP] Connection probe verified!\r\n");
    }
    else
    {
        Debug_Print("\r\n[TCP] Initial probe pending, will retry per transmission.\r\n");
    }

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

        /* -------------------------------------------------------------
         * Step 5: Read Compensated Environmental Data from Sensor API
         * ------------------------------------------------------------- */
        rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);

        if (rslt == BME280_OK)
        {
            temperature = (float)comp_data.temperature;
            pressure = (float)comp_data.pressure / 100.0f; /* Convert Pa to hPa */
            humidity = (float)comp_data.humidity;

            /* ---------------------------------------------------------
             * Step 6: Read Raw 20-bit Pressure and Temperature Registers
             * --------------------------------------------------------- */
            uint8_t measurement_regs[6] = {0};

            HAL_StatusTypeDef read_status = HAL_I2C_Mem_Read(
                &hi2c1,
                (uint16_t)(dev_addr << 1),
                0xF7,
                I2C_MEMADD_SIZE_8BIT,
                measurement_regs,
                6,
                1000
            );

            if (read_status == HAL_OK)
            {
                /* Extract raw 20-bit pressure ADC value */
                uint32_t raw_adc_pressure =
                    ((uint32_t)measurement_regs[0] << 12) |
                    ((uint32_t)measurement_regs[1] << 4)  |
                    ((uint32_t)measurement_regs[2] >> 4);

                /* Extract raw 20-bit temperature ADC value */
                uint32_t raw_adc_temperature =
                    ((uint32_t)measurement_regs[3] << 12) |
                    ((uint32_t)measurement_regs[4] << 4)  |
                    ((uint32_t)measurement_regs[5] >> 4);

                char raw_debug[160];
                int raw_len = snprintf(
                    raw_debug,
                    sizeof(raw_debug),
                    "\r\nRAW ADC PRESSURE    : %lu\r\n"
                    "RAW ADC TEMPERATURE : %lu\r\n"
                    "COMPENSATED PRESSURE: %lu Pa\r\n",
                    (unsigned long)raw_adc_pressure,
                    (unsigned long)raw_adc_temperature,
                    (unsigned long)(comp_data.pressure + 0.5)
                );

                HAL_UART_Transmit(&huart2, (uint8_t *)raw_debug, (uint16_t)raw_len, 1000U);
            }
            else
            {
                Debug_Print("\r\nRAW REGISTER READ FAILED!\r\n");
            }

            /* Display formatted compensated values over USART2 */
            uint32_t raw_pressure_pa = (uint32_t)(comp_data.pressure + 0.5);
            char pressure_debug[128];
            int pressure_len = snprintf(
                pressure_debug,
                sizeof(pressure_debug),
                "Pressure            : %lu.%02lu hPa\r\n",
                (unsigned long)(raw_pressure_pa / 100U),
                (unsigned long)(raw_pressure_pa % 100U)
            );

            if (pressure_len > 0 && pressure_len < (int)sizeof(pressure_debug))
            {
                HAL_UART_Transmit(&huart2, (uint8_t *)pressure_debug, (uint16_t)pressure_len, 1000U);
            }

            /* ---------------------------------------------------------
             * Step 7: Transmit Telemetry to Node-RED via ESP8266 HTTP
             * --------------------------------------------------------- */
            Debug_Print("Transmitting telemetry to Node-RED...\r\n");

            if (SendBME280HTTP(temperature, humidity, pressure))
            {
                Debug_Print(">>> HTTP POST SUCCESSFUL! <<<\r\n");
                HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
            }
            else
            {
                Debug_Print(">>> HTTP POST FAILED! <<<\r\n");
                HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
            }
        }
        else
        {
            Debug_Print("Sensor read error!\r\n");
        }

        HAL_Delay(2000U); /* Poll telemetry cycle every 2 seconds */
    }
    /* USER CODE END 3 */
}

/**
  * @brief  System Clock Configuration (16 MHz HSI)
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK   |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1  |
                                  RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  I2C1 Initialization Function (Standard 100 kHz)
  * @param  None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  USART1 Initialization Function (115200 8N1 - ESP8266)
  * @param  None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  USART2 Initialization Function (115200 8N1 - PC PuTTY Debug)
  * @param  None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  GPIO Initialization Function
  * @param  None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Set default LED level */
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

    /* PC13: User Push Button B1 */
    GPIO_InitStruct.Pin = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

    /* PA5: User LED LD2 */
    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Transmit string over USART2 PC debug interface
  */
static void Debug_Print(const char *text)
{
    HAL_UART_Transmit(
        &huart2,
        (uint8_t *)text,
        (uint16_t)strlen(text),
        1000U
    );
}

/**
  * @brief  Bosch Sensor API I2C read callback using STM32 HAL.
  * @param  reg_addr : 8-bit register address to read from
  * @param  reg_data : Pointer to buffer receiving data
  * @param  len      : Number of bytes to read
  * @param  intf_ptr : Pointer to device I2C 7-bit address
  */
int8_t user_i2c_read(uint8_t reg_addr,
                     uint8_t *reg_data,
                     uint32_t len,
                     void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t *)intf_ptr;

    if (HAL_I2C_Mem_Read(&hi2c1,
                         (uint16_t)(dev_addr << 1),
                         reg_addr,
                         I2C_MEMADD_SIZE_8BIT,
                         reg_data,
                         (uint16_t)len,
                         HAL_MAX_DELAY) == HAL_OK)
    {
        return BME280_OK;
    }

    return BME280_E_COMM_FAIL;
}

/**
  * @brief  Bosch Sensor API I2C write callback using STM32 HAL.
  * @param  reg_addr : 8-bit register address to write to
  * @param  reg_data : Pointer to data to write
  * @param  len      : Number of bytes to write
  * @param  intf_ptr : Pointer to device I2C 7-bit address
  */
int8_t user_i2c_write(uint8_t reg_addr,
                      const uint8_t *reg_data,
                      uint32_t len,
                      void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t *)intf_ptr;

    if (HAL_I2C_Mem_Write(&hi2c1,
                          (uint16_t)(dev_addr << 1),
                          reg_addr,
                          I2C_MEMADD_SIZE_8BIT,
                          (uint8_t *)reg_data,
                          (uint16_t)len,
                          HAL_MAX_DELAY) == HAL_OK)
    {
        return BME280_OK;
    }

    return BME280_E_COMM_FAIL;
}

/**
  * @brief  Bosch Sensor API microsecond delay callback.
  *         Converts microseconds to millisecond SysTick ticks upward.
  */
void user_delay_us(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;
    HAL_Delay((period / 1000U) + 1U);
}

/**
  * @brief  Fixed-point float formatter producing 2 decimal digits without %f.
  */
static void FormatFloat2(char *buffer,
                         uint32_t size,
                         float value)
{
    int32_t scaled;

    if (value >= 0.0f)
        scaled = (int32_t)(value * 100.0f + 0.5f);
    else
        scaled = (int32_t)(value * 100.0f - 0.5f);

    uint32_t magnitude = (scaled < 0)
                       ? (uint32_t)(-scaled)
                       : (uint32_t)scaled;

    snprintf(buffer, size, "%s%lu.%02lu",
             (scaled < 0) ? "-" : "",
             (unsigned long)(magnitude / 100U),
             (unsigned long)(magnitude % 100U));
}

/**
  * @brief  Package environmental data into dynamic JSON and send HTTP POST
  *         to Node-RED analytics server via ESP8266 AT commands.
  */
static uint8_t SendBME280HTTP(float t, float h, float p)
{
    static char tstr[20], hstr[20], pstr[20];
    static char json[160];
    static char request[420];
    static char cmd[80];
    static char response[512];

    uint8_t rx;
    uint16_t count;
    uint32_t start;
    uint8_t prompt_found = 0;
    uint8_t http_ok = 0;

    HAL_StatusTypeDef status;
    int body_len, request_len, cmd_len;

    memset(response, 0, sizeof(response));

    /* 1. Format float values safely */
    FormatFloat2(tstr, sizeof(tstr), t);
    FormatFloat2(hstr, sizeof(hstr), h);
    FormatFloat2(pstr, sizeof(pstr), p);

    /* 2. Construct JSON payload */
    body_len = snprintf(
        json,
        sizeof(json),
        "{\"status\":\"success\",\"temperature\":%s,\"humidity\":%s,\"pressure\":%s}",
        tstr, hstr, pstr
    );

    if (body_len <= 0 || body_len >= (int)sizeof(json))
        return 0U;

    /* 3. Construct HTTP POST request with exact Content-Length */
    request_len = snprintf(
        request,
        sizeof(request),
        "POST /sensor HTTP/1.1\r\n"
        "Host: 10.31.1.46:1880\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n%s",
        body_len,
        json
    );

    if (request_len <= 0 || request_len >= (int)sizeof(request))
        return 0U;

    /* 4. Establish TCP connection to Node-RED */
    cmd_len = snprintf(
        cmd,
        sizeof(cmd),
        "AT+CIPSTART=\"TCP\",\"10.31.1.46\",1880\r\n"
    );

    if (cmd_len <= 0 || cmd_len >= (int)sizeof(cmd))
        return 0U;

    status = HAL_UART_Transmit(
        &huart1,
        (uint8_t *)cmd,
        (uint16_t)cmd_len,
        1000U
    );

    if (status != HAL_OK)
        return 0U;

    /* Wait for TCP connection confirmation */
    memset(response, 0, sizeof(response));
    count = 0;
    start = HAL_GetTick();

    while ((HAL_GetTick() - start) < 10000U &&
           count < sizeof(response) - 1U)
    {
        status = HAL_UART_Receive(&huart1, &rx, 1, 100U);

        if (status == HAL_OK)
        {
            response[count++] = (char)rx;
            response[count] = '\0';

            if (strstr(response, "OK") != NULL ||
                strstr(response, "ALREADY CONNECTED") != NULL)
            {
                break;
            }

            if (strstr(response, "ERROR") != NULL ||
                strstr(response, "FAIL") != NULL)
            {
                break;
            }
        }
        else if (status != HAL_TIMEOUT)
        {
            break;
        }
    }

    if (strstr(response, "OK") == NULL &&
        strstr(response, "ALREADY CONNECTED") == NULL)
    {
        Debug_Print("\r\n[TCP] Connection failed!\r\n");
        Debug_Print(response);
        return 0U;
    }

    /* 5. Request permission to stream data length */
    cmd_len = snprintf(
        cmd,
        sizeof(cmd),
        "AT+CIPSEND=%d\r\n",
        request_len
    );

    if (cmd_len <= 0 || cmd_len >= (int)sizeof(cmd))
        return 0U;

    status = HAL_UART_Transmit(
        &huart1,
        (uint8_t *)cmd,
        (uint16_t)cmd_len,
        1000U
    );

    if (status != HAL_OK)
        return 0U;

    /* 6. Wait for ESP8266 prompt '>' */
    count = 0;
    start = HAL_GetTick();

    while ((HAL_GetTick() - start) < 5000U &&
           count < sizeof(response) - 1U)
    {
        status = HAL_UART_Receive(&huart1, &rx, 1, 100U);

        if (status == HAL_OK)
        {
            if (rx == '>')
            {
                prompt_found = 1;
                break;
            }

            response[count++] = (char)rx;
            response[count] = '\0';

            if (strstr(response, "ERROR") != NULL)
                break;
        }
        else if (status != HAL_TIMEOUT)
        {
            break;
        }
    }

    if (!prompt_found)
    {
        Debug_Print("\r\n[CIPSEND] Prompt '>' timeout!\r\n");
        Debug_Print(response);
        return 0U;
    }

    /* 7. Stream HTTP payload */
    status = HAL_UART_Transmit(
        &huart1,
        (uint8_t *)request,
        (uint16_t)request_len,
        5000U
    );

    if (status != HAL_OK)
        return 0U;

    /* 8. Receive server response */
    memset(response, 0, sizeof(response));
    count = 0;
    start = HAL_GetTick();

    while ((HAL_GetTick() - start) < 10000U &&
           count < sizeof(response) - 1U)
    {
        status = HAL_UART_Receive(&huart1, &rx, 1, 100U);

        if (status == HAL_OK)
        {
            response[count++] = (char)rx;
            response[count] = '\0';

            if (strstr(response, "HTTP/1.1 200") != NULL ||
                strstr(response, "HTTP/1.0 200") != NULL)
            {
                http_ok = 1U;
            }

            if (strstr(response, "CLOSED") != NULL)
            {
                break;
            }
        }
        else if (status != HAL_TIMEOUT)
        {
            break;
        }
    }

    Debug_Print("\r\n--- NODE-RED SERVER RESPONSE ---\r\n");
    Debug_Print(response);
    Debug_Print("\r\n--------------------------------\r\n");

    return http_ok;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
