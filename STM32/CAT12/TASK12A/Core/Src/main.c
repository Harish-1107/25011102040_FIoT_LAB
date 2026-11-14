/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : CAT 12 - TASK 12A: BME280 Sensor Bring-Up, I2C Interface
  *                   and Chip ID Verification using Bosch Sensortec Sensor API
  * @author         : Harish S. (25011102040)
  * @institution    : Shiv Nadar University Chennai
  * @course         : 25CS2741 - Fundamentals of IoT (FIoT) Lab
  ******************************************************************************
  * @attention
  *
  * Hardware Pin Mapping:
  *   I2C1:
  *     PB6 = I2C1_SCL (BME280 SCK)
  *     PB7 = I2C1_SDA (BME280 SDI)
  *   USART2 (PC Serial Debug via ST-LINK VCP @ 115200 baud):
  *     PA2 = USART2_TX (Connects to PC Virtual COM Port RX)
  *     PA3 = USART2_RX (Connects to PC Virtual COM Port TX)
  *   GPIO:
  *     PA5  = User LED LD2 (Heartbeat / Status)
  *     PC13 = User Button B1 (Manual Trigger)
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "bme280.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

struct bme280_dev dev;
int8_t rslt = BME280_OK;
static uint8_t dev_addr = BME280_I2C_ADDR_PRIM; /* 0x76 (SDO -> GND) */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
void user_delay_us(uint32_t period, void *intf_ptr);
static void Debug_Print(const char *text);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();

    const uint8_t banner[] = "\r\n==============================================\r\n"
                             "  TASK 12A: BME280 SENSOR BRING-UP & CHIP ID  \r\n"
                             "  Student: Harish S. (25011102040)            \r\n"
                             "==============================================\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)banner, sizeof(banner) - 1U, 1000U);

    /* 1. Setup Bosch API Hardware Abstraction Function Pointers */
    dev.intf_ptr = &dev_addr;
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;

    /* 2. Initialize BME280 Driver and Read Chip ID from register 0xD0 */
    rslt = bme280_init(&dev);

    char msg[128];
    int len = snprintf(msg, sizeof(msg),
                       "BME280 Init Result : %d\r\n"
                       "Detected Chip ID   : 0x%02X (Expected: 0x60)\r\n",
                       rslt, dev.chip_id);
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, (uint16_t)len, 1000U);

    if (rslt == BME280_OK && dev.chip_id == BME280_CHIP_ID)
    {
        Debug_Print(">> SUCCESS: BME280 I2C Communication Verified! <<\r\n");
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    }
    else
    {
        Debug_Print(">> ERROR: Failed to detect BME280! Check PB6/PB7 wiring. <<\r\n");
    }

    while (1)
    {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        HAL_Delay(1000);
    }
}

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
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

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
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
}

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
    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

static void Debug_Print(const char *text)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)text, (uint16_t)strlen(text), 1000U);
}

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t *)intf_ptr;
    if (HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(dev_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, (uint16_t)len, HAL_MAX_DELAY) == HAL_OK)
        return BME280_OK;
    return BME280_E_COMM_FAIL;
}

int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t *)intf_ptr;
    if (HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(dev_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t *)reg_data, (uint16_t)len, HAL_MAX_DELAY) == HAL_OK)
        return BME280_OK;
    return BME280_E_COMM_FAIL;
}

void user_delay_us(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;
    HAL_Delay((period / 1000U) + 1U);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
