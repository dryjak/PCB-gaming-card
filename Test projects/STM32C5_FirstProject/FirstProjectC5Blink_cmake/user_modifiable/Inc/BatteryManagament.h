#ifndef INC_BATTERYMANAGEMENT_H_
#define INC_BATTERYMANAGEMENT_H_

#include <stdint.h>

//Calculate mean value of ADC data and update battery voltage
void Battery_Update(const uint16_t* adc_buffer, uint16_t buffer_size);

//Getter for battery voltage in millivolts
uint32_t Battery_GetVoltage(void);
//Calculate battery percentage based on voltage
uint8_t Calculate_Battery_Percentage(uint32_t battery_voltage_mv);
//Check if battery is critical
uint8_t Battery_IsCritical(void);

#endif /* INC_BATTERYMANAGEMENT_H_ */
