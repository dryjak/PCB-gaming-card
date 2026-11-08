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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ADC_VOLTAGE_SIZE  10
#define ADC_FULL_SCALE 4095UL
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint16_t adc_voltage[ADC_VOLTAGE_SIZE] = {0};
volatile uint16_t Mean_VBAT = 0;
volatile uint16_t Mean_VREFINT = 0;
uint32_t counter = 0;

volatile uint32_t BatteryVoltage = 0;

/* Private functions prototype -----------------------------------------------*/
void CalculateMeanADC(void);
uint32_t Battery_GetRealMillivolts(void);
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
      //HAL_ADC_Calibration_Start(mx_adc1_gethandle());
      // POPRAWKA 2: Budzimy sprzęt! Kolejność jest krytyczna.
      // 1. Uruchamiamy ADC z asystentem DMA (nasłuchuje na kabel z Timera)
      HAL_ADC_Start(mx_adc1_gethandle());
      HAL_ADC_REG_StartConv_DMA(mx_adc1_gethandle(), (uint8_t*)adc_voltage, ADC_VOLTAGE_SIZE * sizeof(uint16_t));

      // 2. Uruchamiamy metronom (Timer wyzwalający pomiary co 1 sekundę)
      HAL_TIM_Start(mx_tim2_gethandle());
    while (1) 
    {
     // SCB_InvalidateDCache_by_Addr((uint32_t*)adc_voltage, sizeof(adc_voltage));
// POPRAWKA 3: Najpierw każemy kierownikowi wyliczyć średnią z bufora RAM...
      CalculateMeanADC();
      
      // ... a dopiero potem przeliczamy średnią na realne miliwolty.
      BatteryVoltage = Battery_GetRealMillivolts();
      // Małe opóźnienie, aby pętla nie "paliła" CPU milion razy na sekundę,
      // dopóki nie zaimplementujemy flag z DMA (Zadanie domowe).
      counter++;
      HAL_Delay(100);    
    }
  }
} /* end main */


void CalculateMeanADC(void)
{
    uint32_t sum_vbat = 0;
    uint32_t sum_vrefint = 0;

    // Przeskakujemy co 2 elementy (i += 2), ponieważ dane są splecione
    for (int i = 0; i < ADC_VOLTAGE_SIZE; i += 2) 
    {
        sum_vbat    += adc_voltage[i];       // Indeksy parzyste: 0, 2, 4, 6, 8 (Kanał 0)
        sum_vrefint += adc_voltage[i + 1];   // Indeksy nieparzyste: 1, 3, 5, 7, 9 (VREFINT)
    }

    // Dzielimy przez ilość próbek danego typu (5 sztuk)
    Mean_VBAT = sum_vbat / (ADC_VOLTAGE_SIZE / 2);
    Mean_VREFINT = sum_vrefint / (ADC_VOLTAGE_SIZE / 2);
}

uint32_t Battery_GetRealMillivolts(void)
{
    // 1. Zabezpieczenie na wypadek, gdyby DMA jeszcze nie zdążyło przenieść danych
    if (Mean_VREFINT == 0) 
    {
        return 0; 
    }

    // 2. Pobranie unikalnej wartości z ROM przez rzutowanie makra na wskaźnik i wyłuskanie wartości
    uint16_t factory_vrefint_cal = *(__IO uint16_t *)LL_ADC_VREFINT_CAL_ADDR;

    // 3. Obliczenia ze sprzętowym uwzględnieniem spójności typów (promocja do uint64_t w liczniku!)
    // Wzór: V_PIN = (3000mV * Factory_Cal * ADC_BAT) / (ADC_VREFINT * 4095)
    
    uint64_t numerator = (uint64_t)LL_ADC_VREFINT_CAL_VREF * factory_vrefint_cal * Mean_VBAT;
    uint32_t denominator = Mean_VREFINT * ADC_FULL_SCALE;
    
    // Obliczenie napięcia na pinie (tu już wynik bezpiecznie mieści się w uint32_t)
    uint32_t pin_voltage_mv = (uint32_t)(numerator / denominator);

    // 4. Mnożymy x2 z powodu sprzętowego dzielnika (100k / 100k)
    uint32_t actual_battery_mv = pin_voltage_mv * 2;

    return (uint32_t)actual_battery_mv;
}