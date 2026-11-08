/*
 * SSD1306_OLED.c
 *
 *  Created on: Jun 16, 2026
 *      Author: dryla
 */
#include "main.h"
#include "SSD1306_OLED.h"
#include <string.h>

static uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

void SSD1306_Command(SSD1306_t *OLED, uint8_t Command)
{
	HAL_I2C_Mem_Write(OLED->I2c, (OLED->Address<<1), 0x00, 1, &Command, 1, SSD1306_TIMEOUT);
}

void SSD1306_Data(SSD1306_t *OLED, uint8_t *Data, uint16_t Size)
{
	HAL_I2C_Mem_Write(OLED->I2c, (OLED->Address<<1), 0x40, 1, Data, Size, SSD1306_TIMEOUT);
}
//
// Functions
//
void SSD1306_Display(SSD1306_t *OLED)
{
	SSD1306_Command(OLED, SSD1306_PAGEADDR);
	SSD1306_Command(OLED, 0);                   // Page start address
	SSD1306_Command(OLED, 0xFF);                // Page end (not really, but works here)
	SSD1306_Command(OLED, SSD1306_COLUMNADDR); // Column start address
	SSD1306_Command(OLED, 0);                   // Page start address
	SSD1306_Command(OLED, SSD1306_LCDWIDTH - 1); // Column end address

	SSD1306_Data(OLED, SSD1306_Buffer, SSD1306_BUFFER_SIZE);
}

void SSD1306_Clear(uint8_t Color)
{
	switch(Color)
	{
	case SSD1306_WHITE:
		memset(SSD1306_Buffer, 0xFF, SSD1306_BUFFER_SIZE);
	break;

	case SSD1306_BLACK:
		memset(SSD1306_Buffer, 0x00, SSD1306_BUFFER_SIZE);
	break;
	}
}

void SSD1306_DrawPixel(uint16_t x, uint16_t y, uint8_t Color)
{
	if((y <= SSD1306_LCDHEIGHT) && (y > 0) && (x <= SSD1306_LCDWIDTH) && (x > 0))
	{
		switch(Color)
		{
		case SSD1306_WHITE:
			SSD1306_Buffer[x + (y / 8) * SSD1306_LCDWIDTH] |= (1 << (y & 7));
		break;
		case SSD1306_BLACK:
			SSD1306_Buffer[x + (y / 8) * SSD1306_LCDWIDTH] &= ~(1 << (y & 7));
		break;
		case SSD1306_INVERSE:
			SSD1306_Buffer[x + (y / 8) * SSD1306_LCDWIDTH] ^= (1 << (y & 7));
		break;
		}
	}
}

void SSD1306_Init(SSD1306_t *OLED, uint8_t Address, I2C_HandleTypeDef *I2C)
{
	OLED->Address = Address;
	OLED->I2c 	  = I2C;

	SSD1306_Command(OLED, SSD1306_DISPLAYOFF);
	SSD1306_Command(OLED, SSD1306_SETDISPLAYCLOCKDIV);
	SSD1306_Command(OLED, 0x80);
	SSD1306_Command(OLED, SSD1306_SETMULTIPLEX);
	SSD1306_Command(OLED, (uint8_t)(SSD1306_LCDHEIGHT - 1));
	SSD1306_Command(OLED, SSD1306_SETDISPLAYOFFSET);
	SSD1306_Command(OLED, 0x0);
	SSD1306_Command(OLED, SSD1306_SETSTARTLINE);
	SSD1306_Command(OLED, SSD1306_CHARGEPUMP);
	SSD1306_Command(OLED, 0x14);

	SSD1306_Command(OLED, SSD1306_MEMORYMODE);
	SSD1306_Command(OLED, 0x00);
	SSD1306_Command(OLED, SSD1306_SEGREMAP);
	SSD1306_Command(OLED, SSD1306_COMSCANDEC);

	SSD1306_Command(OLED,SSD1306_SETCOMPINS);
	SSD1306_Command(OLED,0x12);
	SSD1306_Command(OLED,SSD1306_SETCONTRAST);
	SSD1306_Command(OLED,0xFF);

	SSD1306_Command(OLED, SSD1306_SETPRECHARGE); // 0xd9
	SSD1306_Command(OLED, 0xF1);

	SSD1306_Command(OLED, SSD1306_SETVCOMDETECT);
	SSD1306_Command(OLED, 0x40);
	SSD1306_Command(OLED, SSD1306_DISPLAYALLON_RESUME);
	SSD1306_Command(OLED, SSD1306_NORMALDISPLAY);
	SSD1306_Command(OLED, SSD1306_DEACTIVATE_SCROLL);

	SSD1306_Command(OLED, SSD1306_DISPLAYON);


}


