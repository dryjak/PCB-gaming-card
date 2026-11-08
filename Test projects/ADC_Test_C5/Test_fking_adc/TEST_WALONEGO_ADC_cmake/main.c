/**
  ******************************************************************************
  * file           : main.c
  * brief          : Main program body
  *                  Calls target system initialization then loop in main.
  ******************************************************************************
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mx_gpio_default.h"
#include "stm32c5xx_hal.h"
#include "stm32c5xx_hal_gpio.h"
#include <stdint.h>

/* Private typedef -----------------------------------------------------------*/
#define ADC_SIZE 5

/*Configure adc*/
#define ADC_RESOLUTION      4095.0f // Max value for 12-bit ADC (2^12 - 1)
#define V_REF               3.3f    // Ref voltage for ADC (3.3V)
#define DIVIDER_RATIO       2.0f    // Voltage divider ratio (assuming equal resistors, Vout = Vin / 2)

/*Lipo 1s*/
#define LIPO_MAX_VOLTAGE    4.2f    // Max voltage for fully charged LiPo battery
#define LIPO_MIN_VOLTAGE    3.2f    // Min safe voltage for LiPo battery

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint16_t AdcData[ADC_SIZE];
uint16_t AdcSum = 0;
uint16_t AdcAverage = 0;
uint8_t BatteryPercentage = 0;

volatile uint8_t DataReadyFlag = 0;

float v_pin = 0.0f; // Voltage at the ADC pin (after voltage divider)
uint8_t BatteryEmptyFlag = 0; // Flag to indicate if battery is empty (below minimum voltage)

uint32_t TimeNow = 0;
/* Private functions prototype -----------------------------------------------*/
uint8_t Calculate_Battery_Percentage(uint16_t adc_value);

/**
  * brief:  The application entry point.
  * retval: none but we specify int to comply with C99 standard
  */
int main(void)
{
  /** System Init: this code placed in targets folder initializes your system.
    * It calls the initialization (and sets the initial configuration) of the peripherals.
    * You can use STM32CubeMX to generate and call this code or not in this project.
    * It also contains the HAL initialization and the initial clock configuration.
    */
  if (mx_system_init() != SYSTEM_OK)
  {
    return (-1);
  }
  else
  {
    /*
      * You can start your application code here
      */
    HAL_ADC_Start(mx_adc1_gethandle());
    HAL_ADC_REG_StartConv_DMA(mx_adc1_gethandle(), (uint8_t*)AdcData, ADC_SIZE * sizeof(uint16_t));
    HAL_TIM_Start(mx_tim2_gethandle());
    
    
    TimeNow = HAL_GetTick();

    while (1) 
    {
/*
      if(HAL_GetTick() - TimeNow >= 1000) // Check if 1 second has passed
      {
        if(BatteryEmptyFlag)
        {
          HAL_GPIO_WritePin(LED_PORT, LED_PIN, 1); // Turn on LED
        }
        else
        {
          HAL_GPIO_WritePin(LED_PORT, LED_PIN, 0); // Turn off LED
        }
        TimeNow = HAL_GetTick(); // Update the time
        // Read ADC values into AdcData array
        for (int i = 0; i < ADC_SIZE; i++)
        {
          AdcSum += AdcData[i]; // Sum the ADC values
        }
        AdcAverage = AdcSum / ADC_SIZE;
        AdcSum = 0; // Reset sum for next iteration

        BatteryPercentage = Calculate_Battery_Percentage(AdcAverage); // Calculate battery percentage
        if (BatteryPercentage <= 50) // Check if battery is below 50%
        {
          BatteryEmptyFlag = 1; // Set flag if battery is low
        }
        else
        {
          BatteryEmptyFlag = 0; // Clear flag if battery is sufficient
        }

      }
  */
      if(DataReadyFlag)
      {
        DataReadyFlag = 0; // Reset the flag
        // Read ADC values into AdcData array
        for (int i = 0; i < ADC_SIZE; i++)
        {
          AdcSum += AdcData[i]; // Sum the ADC values
        }
        AdcAverage = AdcSum / ADC_SIZE;
        AdcSum = 0; // Reset sum for next iteration
      }

      HAL_Delay(10); // Delay for 100 ms to avoid busy waiting
    }
  }
} 


uint8_t Calculate_Battery_Percentage(uint16_t adc_value)
{
    // 1. Obliczenie napięcia na wejściu ADC (za dzielnikiem)
    v_pin = ((float)adc_value / ADC_RESOLUTION) * V_REF;
    
    // 2. Obliczenie rzeczywistego napięcia baterii (przed dzielnikiem)
    float v_battery = v_pin * DIVIDER_RATIO;
    
    // 3. Zabezpieczenie przed wartościami poza zakresem (clamping)
    if (v_battery >= LIPO_MAX_VOLTAGE)
    {
        return 100;
    }
    if (v_battery <= LIPO_MIN_VOLTAGE)
    {
        return 0;
    }
    
    // 4. Obliczenie procentu naładowania (liniowo pomiędzy MIN a MAX)
    float percentage = ((v_battery - LIPO_MIN_VOLTAGE) / (LIPO_MAX_VOLTAGE - LIPO_MIN_VOLTAGE)) * 100.0f;
    
    return (uint8_t)percentage;
}

/* end main */

void HAL_ADC_REG_DataTransferCpltCallback(hal_adc_handle_t *hadc)
{
  if (hadc->instance == mx_adc1_gethandle()->instance)
  {
    // ADC conversion complete callback
    // You can add any additional processing here if needed
    DataReadyFlag = 1;
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN); // Toggle the LED state
  }
}
