#include "BatteryManagement.h"

extern volatile uint16_t Mean; //Mean value of ADC, calculated in main loop

//Lipo 1s
#define LIPO_MAX_VOLTAGE    4.2f    // Max voltage for fully charged LiPo battery
#define LIPO_MIN_VOLTAGE    3.2f    // Min safe voltage for LiPo battery

//ADC thresholds for battery management to calculte TODO:
#define ADC_WARNING_ON      2110  // Próg włączenia ostrzeżenia 
#define ADC_WARNING_OFF     2150  // Próg wyłączenia (HISTEREZA - wyłączamy nieco wyżej!)
#define ADC_CRITICAL_CUTOFF 1950  // Próg wejścia w Standby (Deep Sleep)

#define BATTERY_DEBOUNCE_CYCLES 4 // Liczba cykli odczytu ADC, które muszą być poniżej progu, aby uznać, że bateria jest pusta

uint8_t ShowBatteryWarning = 0;
static uint16_t CriticalVoltageCounter = 0;


void Battery_UpdateLogic(void)
{
    uint16_t current_adc = Mean;

    //Critical voltage check
    if (current_adc < ADC_CRITICAL_CUTOFF)
    {
        CriticalVoltageCounter++;
        if (CriticalVoltageCounter >= BATTERY_DEBOUNCE_CYCLES)
        {
            //If the battery voltage is critically low for a certain number of cycles, execute emergency shutdown.
            // Ta funkcja zablokuje przerwania, zrzuci zasilanie OLED i wpadnie w HAL_PWR_EnterSTANDBYMode()
            System_ExecuteEmergencyShutdown(); 
        }
    }
    else
    {
        //if the voltage is above the critical threshold, reset the counter. This filters out momentary drops under load.
        // resetujemy licznik wyroku. To filtruje chwilowe spadki pod obciążeniem.
        CriticalVoltageCounter = 0;
    }

    //treatment of battery warning flag with hysteresis
    if (ShowBatteryWarning == 0 && current_adc < ADC_WARNING_ON)
    {
        //warning flag is set only when the voltage drops below the threshold. It will not be cleared until the voltage rises above the upper threshold.
        ShowBatteryWarning = 1;
    }
    else if (ShowBatteryWarning == 1 && current_adc > ADC_WARNING_OFF)
    {
        // the warning flag is cleared only when the voltage rises above the upper threshold. This prevents flickering of the warning message.
        ShowBatteryWarning = 0;
    }
}
