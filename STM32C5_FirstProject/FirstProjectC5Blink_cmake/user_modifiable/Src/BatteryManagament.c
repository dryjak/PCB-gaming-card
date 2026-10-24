#include "BatteryManagament.h"
#include "stm32c5xx_ll_adc.h" // Wymagane dla adresów kalibracyjnych fabryki ST
#include <stddef.h>

#define ADC_FULL_SCALE 4095UL

#define LIPO_MIN_VOLTAGE 3200UL // 3.2V w mV
#define LIPO_MAX_VOLTAGE 4200UL // 4.2V w m

#define LIPO_CRITICAL_VOLTAGE 3200UL // 3.20V
/* 
 * ZMIENNE PRYWATNE (static)
 * Niewidoczne dla pliku main.c, nie kolidują z resztą programu.
 */
static uint16_t Mean_VBAT = 0;
static uint16_t Mean_VREFINT = 0;
static uint32_t CurrentBatteryVoltage = 0;

/*
 * FUNKCJE PRYWATNE (static)
 */
// Odpowiada za obliczenie średniej z bufora DMA
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

// Odpowiada za czystą matematykę sprzętową z kompensacją
static uint32_t Battery_CalculateRealMillivolts(void)
{
    if (Mean_VREFINT == 0) return 0; // Zabezpieczenie przed zerem

    uint16_t factory_vrefint_cal = *(__IO uint16_t *)LL_ADC_VREFINT_CAL_ADDR;
    
    // Używamy uint64_t do ochrony przed przepełnieniem podczas mnożenia
    uint64_t numerator = (uint64_t)LL_ADC_VREFINT_CAL_VREF * factory_vrefint_cal * Mean_VBAT;
    uint32_t denominator = Mean_VREFINT * ADC_FULL_SCALE;
    
    uint32_t pin_voltage_mv = (uint32_t)(numerator / denominator);
    
    return pin_voltage_mv * 2; // Dzielnik sprzętowy x2
}

/*
 * FUNKCJE PUBLICZNE (Zdefiniowane w .h)
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
    // Zabezpieczenie przed wartościami poza zakresem
    if (battery_voltage_mv >= LIPO_MAX_VOLTAGE) return 100;
    if (battery_voltage_mv <= LIPO_MIN_VOLTAGE) return 0;

    // Obliczenie procentu naładowania
    float percentage = ((float)(battery_voltage_mv - LIPO_MIN_VOLTAGE) / 
                        (LIPO_MAX_VOLTAGE - LIPO_MIN_VOLTAGE)) * 100.0f;

    return (uint8_t)percentage;
}


uint8_t Battery_IsCritical(void)
{
    // Jeśli napięcie zostało już zmierzone (>0) i spadło poniżej progu, zwróć 1 (PRAWDA)
    if (CurrentBatteryVoltage > 0 && CurrentBatteryVoltage <= LIPO_CRITICAL_VOLTAGE) 
    {
        return 1;
    }
    return 0;
}