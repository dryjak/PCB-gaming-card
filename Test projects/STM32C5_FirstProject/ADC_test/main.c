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
#include "stm32c5xx_drivers/hal/stm32c5xx_hal_i2c.h"
#include "stm32c5xx_hal_adc.h"
#include "stm32c5xx_hal_gpio.h"
#include "stm32c5xx_hal_i2c.h"
#include "mx_i2c1.h"

#include "SSD1306_OLED.h"
#include "GFX_BW.h"
#include "GameConsoleMenu.h"
#include "Button.h"
#include "font_8x5.h"

#include "GameSnake.h"
#include "stm32c5xx_hal_tim.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//table for ADC DMA
uint16_t ADC_Value[10];
//OLED
SSD1306_t OLED; 
uint8_t OledAddress = 0x3C; // OLED I2C address

//Buttons
Button_t Enter, Up, Down, Left, Right;
uint8_t ButtonPressedFlag;

//Game console FSM
GameConsole_t Console;

uint8_t value = 0xAE;
/* Private functions prototype -----------------------------------------------*/
void Action_ChangeContrast();

//Wrapper
void Action_MenuUp(void);
void Action_MenuDown(void);
void Action_MenuEnter(void);
void Action_MenuLeft(void);
void Action_MenuRight(void);

//LED
void TurnLedOff(void);
void TurnLedOn(void);
void ToggleLed(void);

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
      //initialize timer and adc
      HAL_TIM_Start(mx_tim6_gethandle());
       HAL_ADC_REG_StartConv_DMA(mx_adc1_gethandle(), (uint8_t*)ADC_Value, sizeof(ADC_Value));
       //(&HAL_ADC_CHANNEL_6, (uint32_t*)ADC_Value, 10);
      //Initialize OLED
      SSD1306_Init(&OLED, OledAddress, mx_i2c1_i2c_gethandle());
      GFX_SetFont(font_8x5);
      SSD1306_Clear(SSD1306_BLACK);
      HAL_Delay(50);

      //Default screen
      GFX_DrawString(10, 4, "RETRO GAME CONSOLE", WHITE, BLACK);
      GFX_DrawLine(0, 14, 127, 14, WHITE);

      GFX_DrawRectangle(44, 29, 30, 30, WHITE);
      GFX_DrawRectangle(54, 19, 30, 30, WHITE);

      GFX_DrawLine(44, 29, 54, 19, WHITE);
      GFX_DrawLine(73, 29, 83, 19, WHITE);
      GFX_DrawLine(44, 58, 54, 48, WHITE);
      GFX_DrawLine(73, 58, 83, 48, WHITE);

      GFX_DrawFillRectangle(56, 31, 6, 6, WHITE);
      SSD1306_Display(&OLED);
      HAL_Delay(4000);

      //Game console FSM initialization
      Console_Init(&Console);

      //Initialize buttons
      //Initialize Buttons
      ButtonInit(&Up, BUTTON_UP_PORT, BUTTON_UP_PIN, 30, 500, 200);
      ButtonRegisterPressCallback(&Up, Action_MenuUp);
      ButtonRegisterLongPressCallback(&Up, TurnLedOff);
      ButtonRegisterRepeatCallback(&Up, ToggleLed);
      ButtonRegisterGoToIdleCallback(&Up, TurnLedOff);

      ButtonInit(&Enter, BUTTON_ENTER_PORT, BUTTON_ENTER_PIN, 30, 500, 200);
      ButtonRegisterPressCallback(&Enter, Action_MenuEnter);
      ButtonRegisterLongPressCallback(&Enter, TurnLedOff);
      ButtonRegisterRepeatCallback(&Enter, ToggleLed);
      ButtonRegisterGoToIdleCallback(&Enter, TurnLedOff);

      ButtonInit(&Down, BUTTON_DOWN_PORT, BUTTON_DOWN_PIN, 30, 500, 200);
      ButtonRegisterPressCallback(&Down, Action_MenuDown);
      ButtonRegisterLongPressCallback(&Down, TurnLedOff);
      ButtonRegisterRepeatCallback(&Down, ToggleLed);
      ButtonRegisterGoToIdleCallback(&Down, TurnLedOff);

      ButtonInit(&Left, BUTTON_LEFT_PORT, BUTTON_LEFT_PIN, 30, 500, 200);
      ButtonRegisterPressCallback(&Left, Action_MenuLeft);
      ButtonRegisterLongPressCallback(&Left, TurnLedOff);
      ButtonRegisterRepeatCallback(&Left, ToggleLed);
      ButtonRegisterGoToIdleCallback(&Left, TurnLedOff);

      ButtonInit(&Right, BUTTON_RIGHT_PORT, BUTTON_RIGHT_PIN, 30, 500, 200);
      ButtonRegisterPressCallback(&Right, Action_MenuRight);
      ButtonRegisterLongPressCallback(&Right, TurnLedOff);
      ButtonRegisterRepeatCallback(&Right, ToggleLed);
      ButtonRegisterGoToIdleCallback(&Right, TurnLedOff);



      //HAL_I2C_MASTER_Transmit(&hi2c1, (0x3C << 1), 0x00, 1, &value, 1, 100);
      //HAL_I2C_MASTER_Transmit_DMA(&hi2c1, (0x3C << 1), 0x00, 1, &value, 1, 100);
      //HAL_I2C_Memory_Write(&hi2c1, 0x3C << 1, 0x00, 1, &value ,1, 100);
      HAL_GPIO_WritePin(LED_PORT, LED_PIN, HAL_GPIO_PIN_SET);
      HAL_Delay(2000);
      HAL_GPIO_WritePin(LED_PORT, LED_PIN, HAL_GPIO_PIN_RESET);
      HAL_Delay(1000);
    while (1) 
    {
      /*
      if(HAL_GPIO_ReadPin(BUTTON_UP_PORT, BUTTON_UP_PIN) == HAL_GPIO_PIN_RESET || 
      HAL_GPIO_ReadPin(BUTTON_DOWN_PORT, BUTTON_DOWN_PIN) == HAL_GPIO_PIN_RESET ||
      HAL_GPIO_ReadPin(BUTTON_LEFT_PORT, BUTTON_LEFT_PIN) == HAL_GPIO_PIN_RESET ||
      HAL_GPIO_ReadPin(BUTTON_RIGHT_PORT, BUTTON_RIGHT_PIN) == HAL_GPIO_PIN_RESET ||
      HAL_GPIO_ReadPin(BUTTON_ENTER_PORT, BUTTON_ENTER_PIN) == HAL_GPIO_PIN_RESET)
      {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, HAL_GPIO_PIN_SET);
      }
      else
      {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, HAL_GPIO_PIN_RESET);
      }
      HAL_Delay(10);
      */
      
      //Button task
      ButtonTask(&Up);
      ButtonTask(&Down);
      ButtonTask(&Enter);
      ButtonTask(&Left);
      ButtonTask(&Right);

      if (Console.CurrentSystemState == STATE_GAME_SNAKE)
      {
        Snake_UpdateLogic();

        // Rysujemy TYLKO wtedy, gdy logika tego zażąda (ruch lub śmierć)
        if (Snake.NeedsRedraw == 1) {
          Snake_Draw(&OLED);
          Snake.NeedsRedraw = 0; // Opuszczamy flagę
        }
      }
      else
      {
        Console_Draw(&Console, &OLED);
      }

      HAL_Delay(10);
    }
  }
} /* end main */



//Private functions
void Action_ChangeContrast()
{
	    uint32_t temp = Console.Settings[0] * 255;
	    uint8_t hardware_value = (uint8_t)(temp / 100);

	    // Używamy globalnej zmiennej OLED z main.c
	    SSD1306_Command(&OLED, SSD1306_SETCONTRAST);
	    SSD1306_Command(&OLED, hardware_value);
}
void Snake_Confirm(void) {
    if (Snake.IsDead == 1) {
        // Go back to menu
        Console.CurrentSystemState = STATE_GAMES_MENU;
        Console.MenuCursorIndex = 0;
        Console.NeedsRedraw = 1;
    }
}

//change state MENU / game
void Action_PlaySnake(void) {
    // 1. Zmiana stanu systemu na grę
    Console.CurrentSystemState = STATE_GAME_SNAKE;

    // 2. Inicjalizacja gry (wąż ustawia się na środku, SpeedMs domyślnie na 150)
    Snake_Init();

    // 3. NADPISANIE PRĘDKOŚCI NA PODSTAWIE MENU
    // Zmienna Console.Settings[1] przechowuje indeks trybu (0=EASY, 1=NORMAL, 2=HARD)
    switch (Console.Settings[1]) {
        case EASY:
            Snake.SpeedMs = 250;
            break;
        case NORMAL:
            Snake.SpeedMs = 150;
            break;
        case HARD:
            Snake.SpeedMs = 130;
            break;
        default:
            Snake.SpeedMs = 150; // Zabezpieczenie (wartość domyślna)
            break;
    }
}
//wrapper
void Action_MenuUp(void) {
    if (Console.CurrentSystemState == STATE_GAME_SNAKE) {
        Snake_TurnUp();
    }/*
    else if (Console.CurrentSystemState == STATE_GAME_FLAPPY) {
        Flappy_Jump(); // Skok ptaka!
    }*/
    else {
        Console_MoveUp(&Console);
    }
    TurnLedOn();
}
void Action_MenuDown(void) {
    if (Console.CurrentSystemState == STATE_GAME_SNAKE) {
        Snake_TurnDown();
    }/*
    else if (Console.CurrentSystemState == STATE_GAME_FLAPPY) {
        // Puste! Ptak nie reaguje na strzałkę w dół.
    }*/
    else {
        Console_MoveDown(&Console);
    }
    TurnLedOn();
}

void Action_MenuEnter(void) {
    if (Console.CurrentSystemState == STATE_GAME_SNAKE) {
        Snake_Confirm();
    }/*
    else if (Console.CurrentSystemState == STATE_GAME_FLAPPY) {
		// Jeśli ptak zginął, Enter wraca do menu. Jeśli żyje, skacze!
		if (Flappy.IsDead) {
			Console.CurrentSystemState = STATE_GAMES_MENU;
			Console.MenuCursorIndex = 0;
			Console.NeedsRedraw = 1;
		} else {
			Flappy_Jump();
		}
	}*/
    else {
        Console_Enter(&Console);
    }
    TurnLedOn();
}

void Action_MenuLeft(void) {
    if (Console.CurrentSystemState == STATE_GAME_SNAKE) {
        Snake_TurnLeft();
    }/*
    else if (Console.CurrentSystemState == STATE_GAME_FLAPPY) {
        // Puste!
    }*/
    else {
        Console_MoveLeft(&Console);
    }
    TurnLedOn();
}
void Action_MenuRight(void) {
    if (Console.CurrentSystemState == STATE_GAME_SNAKE) {
        Snake_TurnRight();
    }/*
    else if (Console.CurrentSystemState == STATE_GAME_FLAPPY) {
        // Puste!
    }*/
    else {
        Console_MoveRight(&Console);
    }
    TurnLedOn();
}


//basic functions
void ToggleLed(void)
{
	HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}
void TurnLedOff(void)
{
	HAL_GPIO_WritePin(LED_PORT, LED_PIN, HAL_GPIO_PIN_RESET);
	ButtonPressedFlag = 0;
}
void TurnLedOn(void)
{
	HAL_GPIO_WritePin(LED_PORT, LED_PIN, HAL_GPIO_PIN_SET);
	ButtonPressedFlag = 1;
}