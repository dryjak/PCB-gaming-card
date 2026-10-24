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
#include "GameFlappy.h"
#include "BatteryManagament.h"
#include "stm32c5xx_hal_pwr.h"

#include <stdio.h>
#include <string.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/*Configure adc*/
#define ADC_RESOLUTION      4095 // Max value for 12-bit ADC (2^12 - 1)

#define V_REF               3.3f    // Ref voltage for ADC (3.3V)
#define DIVIDER_RATIO       2.0f    // Voltage divider ratio (assuming equal resistors, Vout = Vin / 2)
#define ADC_SIZE 10                 // Number of ADC samples to average for voltage measurement

/*Lipo 1s*/
#define LIPO_MAX_VOLTAGE    4.2f    // Max voltage for fully charged LiPo battery
#define LIPO_MIN_VOLTAGE    3.2f    // Min safe voltage for LiPo battery
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

//table for ADC DMA
volatile uint16_t AdcData[ADC_SIZE] = {0};
uint8_t DataReadyFlag = 0; //ADC data ready flag


uint32_t counter = 0;
volatile uint32_t BatteryVoltage = 0;
uint8_t BatteryPercentage = 0; // Global variable to hold battery percentage

float v_pin = 0.0f; // Voltage at the ADC pin (after voltage divider)
uint8_t BatteryEmptyFlag = 0; // Flag to indicate if battery is empty (below minimum voltage)

volatile uint32_t timer_counter = 0;

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
//batery management
extern uint8_t ShowBatteryWarning; //from BatteryManagement.c
void System_ExecuteEmergencyShutdown(void);


void Action_ChangeContrast();

//ADC mean value calculation
void CalculateMeanADC(void);
uint32_t Battery_GetRealMillivolts(void);


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
    //initialize timer and start adc
    HAL_ADC_Start(mx_adc1_gethandle());
    HAL_ADC_REG_StartConv_DMA(mx_adc1_gethandle(), (uint8_t*)AdcData, ADC_SIZE * sizeof(uint16_t));
    HAL_TIM_Start(mx_tim2_gethandle());

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
    
      //test and blink led
      HAL_GPIO_WritePin(LED_PORT, LED_PIN, HAL_GPIO_PIN_SET);
      HAL_Delay(2000);
      HAL_GPIO_WritePin(LED_PORT, LED_PIN, HAL_GPIO_PIN_RESET);
      HAL_Delay(1000);

      //Debug = 0;

    while (1) 
    {      
      //Button task
      ButtonTask(&Up);
      ButtonTask(&Down);
      ButtonTask(&Enter);
      ButtonTask(&Left);
      ButtonTask(&Right);

      //batery management
        if(DataReadyFlag == 1)
      {
        DataReadyFlag = 0; 
        Battery_Update((uint16_t*)AdcData, ADC_SIZE);
        BatteryVoltage = Battery_GetVoltage();
        BatteryPercentage = Calculate_Battery_Percentage(BatteryVoltage);   

        if (Battery_IsCritical() == 1)
        {
            System_ExecuteEmergencyShutdown();
        }
      }

        //User interface logic
      if (Console.CurrentSystemState == STATE_GAME_SNAKE)
      {
        Snake_UpdateLogic();

        //Draw only when the logic requires it (movement or death)
        if (Snake.NeedsRedraw == 1) {
          Snake_Draw(&OLED);
          Snake.NeedsRedraw = 0; // Clear the flag
        }
      }
      else if (Console.CurrentSystemState == STATE_GAME_FLAPPY)
      {
        Flappy_UpdateLogic();

        //Draw only when the logic requires it (movement or death)
        if (Flappy.NeedsRedraw == 1) {
          Flappy_Draw(&OLED);
          Flappy.NeedsRedraw = 0; //Clear the flag
        }
      }
      else
      {
        Console_Draw(&Console, &OLED);
      }



      //TODO: print battery level warning on OLED
        // if (ShowBatteryWarning == 1)
        // {
        //     // Rysuje małą, inwazyjną flagę w rogu ekranu, żeby gracz wiedział o końcu energii
        //     GFX_DrawFillRectangle(100, 0, 28, 10, BLACK); // Tło dla czytelności
        //     GFX_DrawString(102, 2, "LOW!", WHITE, BLACK);
        //     SSD1306_Display(&OLED); 
        // }




      HAL_Delay(10);

    timer_counter = HAL_TIM_GetCounter(mx_tim2_gethandle());
    }
  }

  
 
} /* end main */



//Private functions

/* for now do not make emergency shutdown
void System_ExecuteEmergencyShutdown(void)
{
    __disable_irq();

    SSD1306_Clear(BLACK);
    GFX_DrawString(10, 20, "BATTERY CRITICAL", WHITE, BLACK);
    GFX_DrawString(15, 40, "TURN OFF DEVICE", WHITE, BLACK);
    SSD1306_Display(&OLED);

    for(volatile uint32_t i = 0; i < 5000000; i++) { __NOP(); }

    SSD1306_Command(&OLED, 0xAE); 

    // 1. Dostęp do domeny Backup (RTC, rejestry podtrzymywane). 
    // W niektórych wersjach HAL2 ta funkcja została przeniesiona do RTC lub zmieniła nazwę.
    // Najczęstszy współczesny odpowiednik to:
    //HAL_PWR_EnableBkUpAccess();

    // 2. Czyszczenie flag wybudzenia (WUF - Wake Up Flags).
    // W nowym HAL-u zazwyczaj czyścimy wszystkie flagi naraz za pomocą funkcji z przyrostkiem Ex (Extended),
    // lub sprecyzowanego makra dla konkretnego pinu (np. PWR_FLAG_WUF1).
    // Spróbuj użyć współczesnego standardu:
#if defined(PWR_WAKEUP_FLAG_ALL)
    __HAL_PWR_CLEAR_WAKEUP_FLAG(PWR_WAKEUP_FLAG_ALL); // Wariant A (Makro)
#else
    // Wariant B (częsty w najnowszych bibliotekach LL/HAL2)
    // HAL_PWREx_ClearWakeupFlag(PWR_FLAG_WUF); 
#endif

    // 3. Wejście w tryb Standby.
    // W HAL2 funkcja ta czasem wymaga jawnego podania sposobu uśpienia krzemu.
    // Jeśli HAL_PWR_EnterSTANDBYMode() wyrzuca błąd "too few arguments", 
    // nowa składnia w HAL2 często wygląda tak:
    // HAL_PWR_EnterSTANDBYMode(PWR_STANDBY_ENTRY_WFI); 
    HAL_PWR_EnterStandbyMode();

    while(1); 
}
*/

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
void Action_PlayFlappy(void) {
    Console.CurrentSystemState = STATE_GAME_FLAPPY;
    Flappy_Init();

    // Opcjonalne sterowanie poziomem trudności we Flappy Bird (szybkość odświeżania klatek)
    switch (Console.Settings[1]) {
        case EASY:
            Flappy.SpeedMs = 50; // Wolniej
            break;
        case NORMAL:
            Flappy.SpeedMs = 40; // Standard
            break;
        case HARD:
            Flappy.SpeedMs = 25; // Szybko
            break;
        default:
            Flappy.SpeedMs = 40; 
            break;
    }
}
//wrapper
void Action_MenuUp(void) {
    if (Console.CurrentSystemState == STATE_GAME_SNAKE) {
        Snake_TurnUp();
    }
    else if (Console.CurrentSystemState == STATE_GAME_FLAPPY) {
        Flappy_Jump(); // Skok ptaka odkomentowany!
    }
    else {
        Console_MoveUp(&Console);
    }
    TurnLedOn();
    //Debug++;
}

void Action_MenuDown(void) {
    if (Console.CurrentSystemState == STATE_GAME_SNAKE) {
        Snake_TurnDown();
    }
    else if (Console.CurrentSystemState == STATE_GAME_FLAPPY) {
        // Puste! Ptak nie reaguje na strzałkę w dół, ale przechwytujemy kliknięcie
    }
    else {
        Console_MoveDown(&Console);
    }
    TurnLedOn();
    //Debug--;
}

void Action_MenuEnter(void) {
    if (Console.CurrentSystemState == STATE_GAME_SNAKE) {
        Snake_Confirm();
    }
    else if (Console.CurrentSystemState == STATE_GAME_FLAPPY) {
        if (Flappy.IsDead) {
            Console.CurrentSystemState = STATE_GAMES_MENU;
            Console.MenuCursorIndex = 0;
            Console.NeedsRedraw = 1;
        } else {
            Flappy_Jump(); // Skok ptaka też pod Enterem
        }
    }
    else {
        Console_Enter(&Console);
    }
    TurnLedOn();
}

void Action_MenuLeft(void) {
    if (Console.CurrentSystemState == STATE_GAME_SNAKE) {
        Snake_TurnLeft();
    }
    else if (Console.CurrentSystemState == STATE_GAME_FLAPPY) {
        // Puste! Przechwytujemy kliknięcie, żeby nie wyjść do menu.
    }
    else {
        Console_MoveLeft(&Console);
    }
    TurnLedOn();
}
void Action_MenuRight(void) {
    if (Console.CurrentSystemState == STATE_GAME_SNAKE) {
        Snake_TurnRight();
    }
    else if (Console.CurrentSystemState == STATE_GAME_FLAPPY) {
        // Puste! Przechwytujemy kliknięcie, żeby nie wejść w ustawienia.
    }
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


//callbacks
void HAL_ADC_REG_DataTransferCpltCallback(hal_adc_handle_t *hadc)
{
  if (hadc->instance == mx_adc1_gethandle()->instance)
  {
    // ADC conversion complete callback
    // You can add any additional processing here if needed
    DataReadyFlag = 1;
    //HAL_GPIO_TogglePin(LED_PORT, LED_PIN); // Toggle the LED state
    Console.NeedsRedraw = 1; // Request a redraw of the console
  }
}



//emergency shutdown
void System_ExecuteEmergencyShutdown(void)
{
    // 1. Zablokowanie przerwań (Zatrzymujemy obsługę Timerów, DMA, itp.)
   

    // 2. Wizualne ostrzeżenie gracza
    SSD1306_Clear(BLACK);
    GFX_DrawString(10, 20, "BATTERY CRITICAL", WHITE, BLACK);
    GFX_DrawString(15, 40, "TURN OFF DEVICE", WHITE, BLACK);
    SSD1306_Display(&OLED);

    // Sprzętowa pętla opóźniająca (~2 sekundy), aby gracz zauważył komunikat
    HAL_Delay(2000);

    // 3. Twarde wygaszenie matrycy OLED
    SSD1306_Command(&OLED, 0xAE); 

    // ------------------ NOWY STANDARD HAL PWR ------------------
    __disable_irq();
    // 4. Zabezpieczenie I/O przed wyciekiem prądu (Zjawisko Floating Pins).
    // Funkcja z linii 618 Twojego pliku. "Zamraża" stan logiczny pinów, 
    // zapobiegając ucieczce prądu przez podciągnięcia ekranu/przycisków.
    HAL_PWR_LP_EnableIORetention(HAL_PWR_IO_RETENTION_ALL);

    // 5. Wyczyszczenie duchów przeszłości.
    // Usuwamy stare flagi Standby/Stop, jeśli układ poprzednio z nich wyszedł.
    // Funkcja z linii 462 Twojego pliku.
    HAL_PWR_CleanPreviousSystemPowerMode();

    // 6. Wyczyszczenie wszystkich flag wybudzenia. 
    // Jeśli jakikolwiek przycisk wybudzający został wciśnięty wcześniej,
    // musimy to anulować, inaczej procesor zaśnie i wybudzi się w tej samej mikrosekundzie.
    // Funkcja z linii 255 Twojego pliku.
    HAL_PWR_LP_CleanWakeupSource(HAL_PWR_WAKEUP_SOURCE_ALL);

    // 7. Śmierć rdzenia i wejście w najgłębszy sen (Wait For Interrupt w domenie Standby)
    // Funkcja z linii 361 Twojego pliku.
    HAL_PWR_EnterStandbyMode();

    // Wykonanie kodu w architekturze ARM nigdy nie powinno dotrzeć do tego miejsca.
    while(1); 
}