#ifndef INC_BATTERYMANAGEMENT_H_
#define INC_BATTERYMANAGEMENT_H_

#include <stdint.h>

// Deklarujemy API - tylko te funkcje będą widoczne dla main.c
// Funkcja parsująca zamek błyskawiczny DMA i licząca absolutne napięcie
void Battery_Update(const uint16_t* adc_buffer, uint16_t buffer_size);

// Funkcja (Getter) zwracająca gotowy wynik do wyświetlenia
uint32_t Battery_GetVoltage(void);
uint8_t Calculate_Battery_Percentage(uint32_t battery_voltage_mv);
uint8_t Battery_IsCritical(void);
#endif /* INC_BATTERYMANAGEMENT_H_ */
