/*
 * GameConsoleMenu.c
 *
 *  Created on: Jun 21, 2026
 *      Author: dryla
 */

#include "GameConsoleMenu.h"

// --- 1. MAKRA POMOCNICZE ---
// Kompilator obliczy rozmiar tablicy. Np. dla MainMenuItems: 3 * (wielkość MenuItem_t) / (wielkość MenuItem_t) = 3
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

extern void Action_ChangeContrast(void);
extern void Action_PlaySnake(void);
extern void Action_PlayFlappy(void);

const MenuItem_t GamesMenuItems[] = {
    {"1. Snake", ITEM_ACTION,	STATE_GAME_SNAKE,    Action_PlaySnake},
    {"2. Flappy bird",  ITEM_ACTION,	STATE_MAIN_MENU,     NULL}, // we do not have game pong
    {"Back",  ITEM_FOLDER,		STATE_MAIN_MENU,     NULL}  //go to main menu
};

const MenuItem_t MainMenuItems[] = {
    {"Games",    ITEM_FOLDER, 	STATE_GAMES_MENU,    NULL},
    {"Settings", ITEM_FOLDER,   STATE_SETTINGS_MENU, NULL},
    {"About",    ITEM_FOLDER,   STATE_INFO_MENU,	 NULL}
    //{"Sleep",    STATE_MAIN_MENU,     null} //Enter_Deep_Sleep_Hardware Wywoła funkcję sprzętową!
};

const MenuItem_t SettingsMenuItems[] = {
    // Text       // Type         // Stan docelowy (dla folderu) // Action
    {"Brightness", ITEM_VALUE,    STATE_SETTINGS_MENU,           Action_ChangeContrast},
    {"Mode",       ITEM_VALUE,    STATE_SETTINGS_MENU,           NULL},
    {"Back",       ITEM_FOLDER,   STATE_MAIN_MENU,               NULL}
};

const GameMode_t GameModes[] = {
	{"EASY",   EASY},
	{"NORMAL", NORMAL},
	{"HARD",   HARD}
};

/**
 * @brief Initialize console to basic state
 */
void Console_Init(GameConsole_t *Console) {
    Console->CurrentSystemState = STATE_MAIN_MENU;
    Console->MenuCursorIndex = 0;
    Console->NeedsRedraw = 1;

    Console->SettingsMaxValues[0] = 100;	//max value of brightness
    Console->SettingsMaxValues[1] = 2;		//max value of game modes
    Console->SettingsMinValues[0] = 10;		//min value of brightness
    Console->SettingsMinValues[1] = 0;		//min value of game modes

    Console->Settings[0] = 100;
    Console->Settings[1] = NORMAL;
}

// --- 3. LOGIKA SILNIKA MENU ---

/**
 * @brief Funkcja wewnętrzna (Bridge). Zwraca tablicę i jej rozmiar dla aktualnego stanu.
 */
static void Get_Active_Menu_Data(GameConsoleState_t State, const MenuItem_t** OutMenuArray, uint8_t* OutSize) {
    switch(State) {
        case STATE_MAIN_MENU:
            *OutMenuArray = MainMenuItems;
            *OutSize = ARRAY_SIZE(MainMenuItems);
            break;
        case STATE_SETTINGS_MENU:
			*OutMenuArray = SettingsMenuItems;
			*OutSize = ARRAY_SIZE(SettingsMenuItems);
			break;
        case STATE_GAMES_MENU:
            *OutMenuArray = GamesMenuItems;
            *OutSize = ARRAY_SIZE(GamesMenuItems);
            break;
        default:
            // Zabezpieczenie przed błędem - domyślnie wracamy do głównego menu
            *OutMenuArray = MainMenuItems;
            *OutSize = ARRAY_SIZE(MainMenuItems);
            break;
    }
}

/**
 * @brief Reakcja na przycisk "W DÓŁ".
 */
void Console_MoveRight(GameConsole_t *Console)
{
    const MenuItem_t *CurrentMenu;
    uint8_t MenuSize;
    Get_Active_Menu_Data(Console->CurrentSystemState, &CurrentMenu, &MenuSize);
    MenuItem_t SelectedItem = CurrentMenu[Console->MenuCursorIndex];

    if(Console->IsEditMode == 0)
    {
        // Tryb przeglądania
        if (SelectedItem.NextState != STATE_MAIN_MENU && SelectedItem.Type == ITEM_FOLDER)
        {
            Console->CurrentSystemState = SelectedItem.NextState;
            Console->MenuCursorIndex = 0;
            Console->NeedsRedraw = 1;
        }
    }
    else
    {
        // Tryb edycji
        uint8_t idx = Console->MenuCursorIndex;
        uint8_t step = (idx == 0) ? 10 : 1; // Skok: 10 dla jasności, 1 dla reszty

        // Sprawdzamy "przyszłość": Czy po dodaniu nie przekroczymy maksimum?
        if((Console->Settings[idx] + step) <= Console->SettingsMaxValues[idx])
        {
            Console->Settings[idx] += step;
        }
        else
        {
            // Jeśli przekroczymy, zawijamy na minimum
            Console->Settings[idx] = Console->SettingsMinValues[idx];
        }

        // Akcja na żywo (z zabezpieczeniem przed HardFault!)
        if (SelectedItem.Action != NULL) {
            SelectedItem.Action();
        }

        Console->NeedsRedraw = 1;
    }
}

void Console_MoveLeft(GameConsole_t *Console)
{
    const MenuItem_t *CurrentMenu;
    uint8_t MenuSize;
    Get_Active_Menu_Data(Console->CurrentSystemState, &CurrentMenu, &MenuSize);
    MenuItem_t SelectedItem = CurrentMenu[Console->MenuCursorIndex];

    if(Console->IsEditMode == 0)
    {
        // Globalny powrót
        if (Console->CurrentSystemState != STATE_MAIN_MENU) {
            Console->CurrentSystemState = STATE_MAIN_MENU;
            Console->MenuCursorIndex = 0;
            Console->NeedsRedraw = 1;
        }
    }
    else
    {
        // Tryb edycji
        uint8_t idx = Console->MenuCursorIndex;
        uint8_t step = (idx == 0) ? 10 : 1;

        // Sprawdzamy "przyszłość" przed odjęciem (uwaga na underflow dla uint8_t!)
        if(Console->Settings[idx] >= (Console->SettingsMinValues[idx] + step))
        {
            Console->Settings[idx] -= step;
        }
        else
        {
            // Jeśli zabrakło nam miejsca w dół, zawijamy na maksimum
            Console->Settings[idx] = Console->SettingsMaxValues[idx];
        }

        // Akcja na żywo
        if (SelectedItem.Action != NULL) {
            SelectedItem.Action();
        }

        Console->NeedsRedraw = 1;
    }
}

void Console_MoveDown(GameConsole_t *Console) {
	if (Console->IsEditMode == 1) return;	//stop cursor if in edit mode

    const MenuItem_t *CurrentMenu;
    uint8_t MenuSize;

    // Pobieramy informacje o aktualnym pokoju, w którym jesteśmy
    Get_Active_Menu_Data(Console->CurrentSystemState, &CurrentMenu, &MenuSize);

    // Zabezpieczenie i zapętlenie kursora
    if (Console->MenuCursorIndex < (MenuSize - 1)) {
    	Console->MenuCursorIndex++; // Zjedź niżej
    } else {
    	Console->MenuCursorIndex = 0; // Jesteśmy na dole, wróć na samą górę!
    }

    Console->NeedsRedraw = 1;

}

void Console_MoveUp(GameConsole_t *Console) {
	if (Console->IsEditMode == 1) return;

    const MenuItem_t *CurrentMenu;
    uint8_t MenuSize;

    Get_Active_Menu_Data(Console->CurrentSystemState, &CurrentMenu, &MenuSize);

    // Zabezpieczenie i zapętlenie kursora
    if (Console->MenuCursorIndex > 0) {
    	Console->MenuCursorIndex--; // go up
    }
    else
    {
    	Console->MenuCursorIndex = MenuSize - 1;
    }

    Console->NeedsRedraw = 1;
}

/**
 * @brief Reakcja na przycisk "ENTER". Wykonuje akcję i zmienia stan.
 */
void Console_Enter(GameConsole_t *Console) {
    const MenuItem_t *CurrentMenu;
    uint8_t MenuSize;

    Get_Active_Menu_Data(Console->CurrentSystemState, &CurrentMenu, &MenuSize);

    // Pobieramy aktualny element
        MenuItem_t SelectedItem = CurrentMenu[Console->MenuCursorIndex];

        // Jeśli jesteśmy w trybie edycji, Enter z niego wychodzi (zatwierdza)
        if (Console->IsEditMode == 1) {
            Console->IsEditMode = 0;

            // NOWOŚĆ: Jeśli opcja ma przypisaną akcję sprzętową, wykonaj ją teraz!
			if (SelectedItem.Action != NULL)
			{
				SelectedItem.Action();
			}
            Console->NeedsRedraw = 1;
            return;
        }

        // Sprawdzamy typ podświetlonego elementu
        if (SelectedItem.Type == ITEM_VALUE) {
            // Wchodzimy w tryb edycji!
            Console->IsEditMode = 1;
            Console->NeedsRedraw = 1;
        }
        else if (SelectedItem.Type == ITEM_FOLDER) {
            // Przechodzimy do innego menu
            Console->CurrentSystemState = SelectedItem.NextState;
            Console->MenuCursorIndex = 0;
            Console->NeedsRedraw = 1;
        }
        else if (SelectedItem.Type == ITEM_ACTION) {
            // Wykonujemy akcję (np. start gry)
            if (SelectedItem.Action != NULL) SelectedItem.Action();
        }
}


void Console_Draw(GameConsole_t *Console, SSD1306_t *Display)
{
	if(Console->NeedsRedraw != 1)
	{
		return;
	}
	Console->NeedsRedraw = 0;

	const MenuItem_t *CurrentMenu; 		//current menu to show
	uint8_t CurrentMenuSize;

	Get_Active_Menu_Data(Console->CurrentSystemState, &CurrentMenu, &CurrentMenuSize);

	//clear the display
	SSD1306_Clear(BLACK);

	//switch case to display titles of pages
	uint8_t y_pos = 1;
	switch (Console->CurrentSystemState)
	{
		case STATE_MAIN_MENU:		//printf title main menu
			GFX_DrawString(13, y_pos, "--- MAIN MENU ---",  WHITE, 0);
		break;
		case STATE_GAMES_MENU:		//printf games title
			GFX_DrawString(25, y_pos, "--- GAMES ---",  WHITE, 0);
		break;
		case STATE_SETTINGS_MENU:	//printf settings title
			GFX_DrawString(16, y_pos, "--- SETTINGS ---",  WHITE, 0);

		break;
		case STATE_INFO_MENU:		//printf info title
			GFX_DrawString(25, y_pos, "--- ABOUT ---",  WHITE, 0);
		break;
	}

	//Display page all
		for (int8_t i = 0; i < CurrentMenuSize; i++)
		{
			y_pos = 15 + (i * 12);

			// Rysujemy wskaźnik (kursor)
			if (i == Console->MenuCursorIndex)
			{
				// Jeśli jesteśmy w trybie edycji, rysujemy mały kwadracik lub znak zachęty
				if (Console->IsEditMode) {
					GFX_DrawChar(2, y_pos, '*', WHITE, BLACK); // Tryb edycji
				} else {
					GFX_DrawChar(2, y_pos, '>', WHITE, BLACK); // Zwykły kursor
				}
			}

			// RYSOWANIE ZAWARTOŚCI (Magia formatowania)
			if (CurrentMenu[i].Type == ITEM_VALUE)
			{
				// Tworzymy tymczasowy bufor na sformatowany tekst (np. max 20 znaków)
				char ValueBuffer[20];

				// Zabezpieczamy się: Sprawdzamy, którą opcję z ITEM_VALUE właśnie rysujemy
				if (i == 0) // Brightness (indeks 0 w SettingsMenuItems)
				{
					snprintf(ValueBuffer, sizeof(ValueBuffer), "%s: %d", CurrentMenu[i].Text, Console->Settings[0]);
				}
				else if (i == 1) // Mode (indeks 1 w SettingsMenuItems)
				{
					// Zamiast liczby 0,1,2, bierzemy tekst "EASY", "NORMAL" z naszej nowej tablicy!
					uint8_t mode_index = Console->Settings[1];
					snprintf(ValueBuffer, sizeof(ValueBuffer), "%s: %s", CurrentMenu[i].Text, GameModes[mode_index].Text);
				}

				// Rysujemy nasz nowy, sklejony tekst na ekranie!
				GFX_DrawString(15, y_pos, ValueBuffer, WHITE, BLACK);
			}
			else
			{
				// Jeśli to zwykły folder (np. "Games" lub "Back"), rysujemy go normalnie z Flash
				GFX_DrawString(15, y_pos, (char*) CurrentMenu[i].Text, WHITE, BLACK);
			}
		}

		SSD1306_Display(Display);
}

