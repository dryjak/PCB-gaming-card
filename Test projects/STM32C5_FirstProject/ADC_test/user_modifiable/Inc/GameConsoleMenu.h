/*
 * GameMenu.h
 *
 *  Created on: Jun 21, 2026
 *      Author: dryla
 */

#ifndef INC_GAMECONSOLEMENU_H_
#define INC_GAMECONSOLEMENU_H_

#include <stdint.h>
#include <stddef.h>
#include <SSD1306_OLED.h>
#include <GFX_BW.h>
#include <stdio.h>

typedef enum{
	STATE_MAIN_MENU = 0,
	STATE_GAMES_MENU,
	STATE_SETTINGS_MENU,
	STATE_INFO_MENU,
	STATE_GAME_SNAKE,
	STATE_GAME_FLAPPY,
}GameConsoleState_t;

typedef void (*MenuAction_t) (void);

typedef enum {
    ITEM_FOLDER, // go inside different menu
    ITEM_ACTION, // function e.g. go play snake
    ITEM_VALUE   // edit parameter e.g. brightness
} MenuItemType_t;

typedef struct
{
	const char* Text;
	MenuItemType_t Type;
	GameConsoleState_t NextState;
	MenuAction_t Action;
}MenuItem_t;

typedef enum
{
	EASY = 0,
	NORMAL,
	HARD
}GameModeState_t;

typedef struct
{
	const char* Text;
	GameModeState_t GameMode;
}GameMode_t;
// Pick game menu
extern const MenuItem_t GamesMenuItems[];

// Main Menu
extern const MenuItem_t MainMenuItems[];

typedef struct
{
	GameConsoleState_t CurrentSystemState;
	uint8_t MenuCursorIndex;

	uint8_t NeedsRedraw;
	uint8_t IsEditMode; // < 0 = just read, 1 = edit

	uint8_t Settings[2]; // 0 - brightness, 1 - edit
	uint8_t SettingsMaxValues[2];// brightness - 100, 1 - edit- 3
	uint8_t SettingsMinValues[2];

}GameConsole_t;


void Console_Enter(GameConsole_t *Console);
void Console_MoveUp(GameConsole_t *Console);
void Console_MoveDown(GameConsole_t *Console);
void Console_MoveLeft(GameConsole_t *Console);
void Console_MoveRight(GameConsole_t *Console);

void Console_Init(GameConsole_t *Console);
void Console_Draw(GameConsole_t *Console, SSD1306_t *Display);


#endif /* INC_GAMECONSOLEMENU_H_ */
