#include "BatteryManagament.h"
#include "stm32c5xx_ll_adc.h" // Wymagane dla adresów kalibracyjnych fabryki ST
#include <stddef.h>

#define ADC_FULL_SCALE 4095UL

#define LIPO_MIN_VOLTAGE 3200UL // 3.2V w mV
#define LIPO_MAX_VOLTAGE 4200UL // 4.2V w m

#define LIPO_CRITICAL_VOLTAGE 3200UL // 3.20V
/* 
 *static variables to hold mean values and current battery voltage   
 */
static uint16_t Mean_VBAT = 0;
static uint16_t Mean_VREFINT = 0;
static uint32_t CurrentBatteryVoltage = 0;

/*
 * PRIVATE FUNCTIONS
 */
// Calculate mean values of ADC data for battery and VREFINT channels
static void CalculateMeanADC(const uint16_t* adc_buffer, uint16_t buffer_size)
{
    uint32_t sum_vbat = 0;
    uint32_t sum_vrefint = 0;

    // Przeskakujemy co 2 elementy - dane są splecione
    for (uint16_t i = 0; i < buffer_size; i += 2) 
    {
        sum_vbat    += adc_buffer[i];       // Indeksy parzyste: Kanał 0 (Bateria)
        sum_vrefint += adc_buffer[i + 1];   // Indeksy nieparzyste: VREFINT
    }

    Mean_VBAT = sum_vbat / (buffer_size / 2);
    Mean_VREFINT = sum_vrefint / (buffer_size / 2);
}

// Calculate the real battery voltage in millivolts based on mean ADC values    
static uint32_t Battery_CalculateRealMillivolts(void)
{
    if (Mean_VREFINT == 0) return 0; //defensive programming - avoid division by zero

    uint16_t factory_vrefint_cal = *(__IO uint16_t *)LL_ADC_VREFINT_CAL_ADDR;
    
    uint64_t numerator = (uint64_t)LL_ADC_VREFINT_CAL_VREF * factory_vrefint_cal * Mean_VBAT;
    uint32_t denominator = Mean_VREFINT * ADC_FULL_SCALE;
    
    uint32_t pin_voltage_mv = (uint32_t)(numerator / denominator);
    
    return pin_voltage_mv * 2; // voltage devider ratio is 2:1, so multiply by 2 to get the actual battery voltage
}

/*
 * PUBLIC FUNCTIONS
 */
void Battery_Update(const uint16_t* adc_buffer, uint16_t buffer_size)
{
    // Zabezpieczenie przed pustym wskaźnikiem
    if (adc_buffer == NULL || buffer_size == 0) return;

    CalculateMeanADC(adc_buffer, buffer_size);
    CurrentBatteryVoltage = Battery_CalculateRealMillivolts();
}

uint32_t Battery_GetVoltage(void)
{
    return CurrentBatteryVoltage;
}

uint8_t Calculate_Battery_Percentage(uint32_t battery_voltage_mv)
{
    // defend against out-of-range values
    if (battery_voltage_mv >= LIPO_MAX_VOLTAGE) return 100;
    if (battery_voltage_mv <= LIPO_MIN_VOLTAGE) return 0;

    // battery percentage calculation based on linear interpolation between min and max voltage
    float percentage = ((float)(battery_voltage_mv - LIPO_MIN_VOLTAGE) / 
                        (LIPO_MAX_VOLTAGE - LIPO_MIN_VOLTAGE)) * 100.0f;

    return (uint8_t)percentage;
}


uint8_t Battery_IsCritical(void)
{
    // defend against out-of-range values
    if (CurrentBatteryVoltage == 0) return 0;

    // If the battery voltage has been measured and is below the critical threshold, return 1 (TRUE)
    if (CurrentBatteryVoltage <= LIPO_CRITICAL_VOLTAGE) 
    {
        return 1;
    }
    return 0;
}