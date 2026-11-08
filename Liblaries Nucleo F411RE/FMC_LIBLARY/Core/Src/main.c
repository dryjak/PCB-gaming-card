/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SSD1306_OLED.h"
#include "GFX_BW.h"
#include "Button.h"
#include "GameConsoleMenu.h"
#include "fonts/fonts.h"
#include "GameSnake.h"
#include "FlappyBird.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
SSD1306_t OLED;

//Buttons
Button_t Enter, Up, Down, Left, Right;
uint8_t ButtonPressedFlag;

//Game console FSM
GameConsole_t Console;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
void Snake_Confirm(void);
void Action_PlaySnake(void);

void Action_PlayFlappy(void);

void TurnLedOff(void);
void TurnLedOn(void);
void ToggleLed(void);

//Wrapper
void Action_MenuUp(void);
void Action_MenuDown(void);
void Action_MenuEnter(void);
void Action_MenuLeft(void);
void Action_MenuRight(void);




/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  SSD1306_Init(&OLED, 0x3C, &hi2c1);
  HAL_Delay(50);
  GFX_SetFont(font_8x5);
  SSD1306_Clear(BLACK);
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
//  Console_Draw(&Console, &OLED);

  //Initialize Buttons
  ButtonInit(&Up, ButtonUp_GPIO_Port, ButtonUp_Pin, 30, 500, 200);
  ButtonRegisterPressCallback(&Up, Action_MenuUp);
  ButtonRegisterLongPressCallback(&Up, TurnLedOff);
  ButtonRegisterRepeatCallback(&Up, ToggleLed);
  ButtonRegisterGoToIdleCallback(&Up, TurnLedOff);

  ButtonInit(&Enter, ButtonEnter_GPIO_Port, ButtonEnter_Pin, 30, 500, 200);
  ButtonRegisterPressCallback(&Enter, Action_MenuEnter);
  ButtonRegisterLongPressCallback(&Enter, TurnLedOff);
  ButtonRegisterRepeatCallback(&Enter, ToggleLed);
  ButtonRegisterGoToIdleCallback(&Enter, TurnLedOff);

  ButtonInit(&Down, ButtonDown_GPIO_Port, ButtonDown_Pin, 30, 500, 200);
  ButtonRegisterPressCallback(&Down, Action_MenuDown);
  ButtonRegisterLongPressCallback(&Down, TurnLedOff);
  ButtonRegisterRepeatCallback(&Down, ToggleLed);
  ButtonRegisterGoToIdleCallback(&Down, TurnLedOff);

  ButtonInit(&Left, ButtonLeft_GPIO_Port, ButtonLeft_Pin, 30, 500, 200);
  ButtonRegisterPressCallback(&Left, Action_MenuLeft);
  ButtonRegisterLongPressCallback(&Left, TurnLedOff);
  ButtonRegisterRepeatCallback(&Left, ToggleLed);
  ButtonRegisterGoToIdleCallback(&Left, TurnLedOff);

  ButtonInit(&Right, ButtonRight_GPIO_Port, ButtonRight_Pin, 30, 500, 200);
  ButtonRegisterPressCallback(&Right, Action_MenuRight);
  ButtonRegisterLongPressCallback(&Right, TurnLedOff);
  ButtonRegisterRepeatCallback(&Right, ToggleLed);
  ButtonRegisterGoToIdleCallback(&Right, TurnLedOff);


//DONE TODO: Add variable that shows us current brightness and mode in settings section
//TODO: Add About section
//DONE TODO: Add function for left and right button
 //TODO: Add second game
 //TODO: do electronic schematic in kicad
 //TODO: how to save settingss changed by user?
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	ButtonTask(&Up);
	ButtonTask(&Down);
	ButtonTask(&Enter);
	ButtonTask(&Left);
	ButtonTask(&Right);

/*
	if(HAL_GPIO_ReadPin(ButtonEnter_GPIO_Port, ButtonEnter_Pin) == GPIO_PIN_RESET
		  || HAL_GPIO_ReadPin(ButtonUp_GPIO_Port, ButtonUp_Pin) == GPIO_PIN_RESET
		  || HAL_GPIO_ReadPin(ButtonDown_GPIO_Port, ButtonDown_Pin) == GPIO_PIN_RESET
		  || HAL_GPIO_ReadPin(ButtonLeft_GPIO_Port, ButtonLeft_Pin) == GPIO_PIN_RESET
		  || HAL_GPIO_ReadPin(ButtonRight_GPIO_Port, ButtonRight_Pin) == GPIO_PIN_RESET)
	{
	  HAL_GPIO_WritePin(LedRed_GPIO_Port, LedRed_Pin, GPIO_PIN_SET);
	}
	else
	{
	  HAL_GPIO_WritePin(LedRed_GPIO_Port, LedRed_Pin, GPIO_PIN_RESET);
	}
	*/

	if (Console.CurrentSystemState == STATE_GAME_SNAKE)
	{
		Snake_UpdateLogic();

		// Rysujemy TYLKO wtedy, gdy logika tego zażąda (ruch lub śmierć)
		if (Snake.NeedsRedraw == 1) {
			Snake_Draw(&OLED);
			Snake.NeedsRedraw = 0; // Opuszczamy flagę
		}
	}/*
	else if(Console.CurrentSystemState == STATE_GAME_FLAPPY)
	{
		Flappy_UpdateLogic();

		if (Flappy.NeedsRedraw == 1)
		{
			Flappy_Draw(&OLED);
			Flappy.NeedsRedraw = 0;
		}
	}*/
	else
	{
		Console_Draw(&Console, &OLED);
	}

	HAL_Delay(10);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* I2C1_EV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
}

/* USER CODE BEGIN 4 */
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
/*
void Action_PlayFlappy(void) {
    // 1. Zmiana stanu całego urządzenia na grę Flappy Bird
    Console.CurrentSystemState = STATE_GAME_FLAPPY;

    // 2. Zresetowanie fizyki (ptak ląduje na środku, rury się ustawiają)
    Flappy_Init();

    // 3. Konfiguracja poziomu trudności na podstawie opcji w Settings
    // Console.Settings[1] to nasz tryb (0=EASY, 1=NORMAL, 2=HARD)
    switch (Console.Settings[1]) {
        case EASY:
            Flappy.FrameRateMs = 40; // Wolniejsza fizyka - łatwiej reagować
            break;
        case NORMAL:
            Flappy.FrameRateMs = 30; // Standardowa prędkość
            break;
        case HARD:
            Flappy.FrameRateMs = 20; // Szybsza fizyka - mało czasu na reakcję!
            break;
        default:
            Flappy.FrameRateMs = 30;
            break;
    }
}
*/
//change display brightness
void Action_ChangeContrast()
{
	    uint32_t temp = Console.Settings[0] * 255;
	    uint8_t hardware_value = (uint8_t)(temp / 100);

	    // Używamy globalnej zmiennej OLED z main.c
	    SSD1306_Command(&OLED, SSD1306_SETCONTRAST);
	    SSD1306_Command(&OLED, hardware_value);
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
	HAL_GPIO_TogglePin(LedRed_GPIO_Port, LedRed_Pin);
}
void TurnLedOff(void)
{
	HAL_GPIO_WritePin(LedRed_GPIO_Port, LedRed_Pin, GPIO_PIN_RESET);
	ButtonPressedFlag = 0;
}
void TurnLedOn(void)
{
	HAL_GPIO_WritePin(LedRed_GPIO_Port, LedRed_Pin, GPIO_PIN_SET);
	ButtonPressedFlag = 1;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
