/*
 * GameSnake.c
 *
 *  Created on: Jun 28, 2026
 *      Author: dryla
 */

#include "GameSnake.h"
#include <stdio.h>

// Snake global instance
GameSnake_t Snake;

// random position of new apple
static void SpawnApple(void) {
    Snake.Apple.x = rand() % SNAKE_GRID_WIDTH;
    Snake.Apple.y = rand() % SNAKE_GRID_HEIGHT;
    // We do not check if the apple appeared in the snake body
}

void Snake_Init(void) {
    // Reset start parameters
    Snake.Length = 3;
    Snake.Direction = DIR_RIGHT;
    Snake.IsDead = 0;
    Snake.SpeedMs = 150; // speed of the snake, you can pick mode hard, easy, normal in settings
    Snake.Score = 0;
    Snake.LastMoveTime = HAL_GetTick();

    // Start position of the snake
    Snake.Body[0].x = SNAKE_GRID_WIDTH / 2;
    Snake.Body[0].y = SNAKE_GRID_HEIGHT / 2;

    // generating tail
    Snake.Body[1].x = Snake.Body[0].x - 1;
    Snake.Body[1].y = Snake.Body[0].y;
    Snake.Body[2].x = Snake.Body[0].x - 2;
    Snake.Body[2].y = Snake.Body[0].y;

    srand(HAL_GetTick());
    SpawnApple();
    Snake.NeedsRedraw = 1;
}

void Snake_TurnUp(void)    { if(Snake.Direction != DIR_DOWN)  Snake.Direction = DIR_UP; }
void Snake_TurnDown(void)  { if(Snake.Direction != DIR_UP)    Snake.Direction = DIR_DOWN; }
void Snake_TurnLeft(void)  { if(Snake.Direction != DIR_RIGHT) Snake.Direction = DIR_LEFT; }
void Snake_TurnRight(void){ if(Snake.Direction != DIR_LEFT)  Snake.Direction = DIR_RIGHT; }

void Snake_UpdateLogic(void)
{
    if (Snake.IsDead) return;

        if (HAL_GetTick() - Snake.LastMoveTime < Snake.SpeedMs) {
            return;
        }
        Snake.LastMoveTime = HAL_GetTick();

        //snake moved, draw new pixel
        Snake.NeedsRedraw = 1;

    // 1. Moving the snake body
    for (int i = Snake.Length - 1; i > 0; i--) {
        Snake.Body[i] = Snake.Body[i - 1];
    }

    // 2. Moving in proper direction
    switch (Snake.Direction) {
        case DIR_UP:    Snake.Body[0].y--; break;
        case DIR_DOWN:  Snake.Body[0].y++; break;
        case DIR_LEFT:  Snake.Body[0].x--; break;
        case DIR_RIGHT: Snake.Body[0].x++; break;
    }

    // 3. Wall interferance
        if (Snake.Body[0].x < 0 || Snake.Body[0].x >= SNAKE_GRID_WIDTH ||
            Snake.Body[0].y < 0 || Snake.Body[0].y >= SNAKE_GRID_HEIGHT) {

            Snake.Body[0] = Snake.Body[1];

            Snake.IsDead = 1;
            Snake.NeedsRedraw = 1;
            return;
        }

        // 4. snake eats its own tail
        for (int i = 1; i < Snake.Length; i++) {
            if (Snake.Body[0].x == Snake.Body[i].x && Snake.Body[0].y == Snake.Body[i].y) {
                Snake.IsDead = 1;
                Snake.NeedsRedraw = 1;
                return;
            }
        }

    // 5. Eating the apple
        if (Snake.Body[0].x == Snake.Apple.x && Snake.Body[0].y == Snake.Apple.y) {
        	Snake.Score++;

            if (Snake.Length < SNAKE_MAX_LENGTH) {

                Snake.Body[Snake.Length].x = Snake.Body[Snake.Length - 1].x;
                Snake.Body[Snake.Length].y = Snake.Body[Snake.Length - 1].y;

                // Increasing snake dimensions
                Snake.Length++;

                // gently increase speed
                if(Snake.SpeedMs > 50) Snake.SpeedMs -= 5;
            }
            SpawnApple();
        }
}

void Snake_Draw(SSD1306_t *Display) {
    SSD1306_Clear(BLACK);

    // Draw apple
    GFX_DrawFillRectangle(
        Snake.Apple.x * SNAKE_BLOCK_SIZE,
        Snake.Apple.y * SNAKE_BLOCK_SIZE,
        SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, WHITE);

    // Draw snake
    for (int i = 0; i < Snake.Length; i++) {
        GFX_DrawFillRectangle(
            Snake.Body[i].x * SNAKE_BLOCK_SIZE,
            Snake.Body[i].y * SNAKE_BLOCK_SIZE,
            SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, WHITE);
    }

    // Draw end game - dead
        if (Snake.IsDead) {
            // Title
            GFX_DrawString(37, 15, "GAME OVER", WHITE, BLACK);

            char scoreText[20];
            snprintf(scoreText, sizeof(scoreText), "Score: %d", Snake.Score);

            // Write score
            GFX_DrawString(37, 27 , scoreText, WHITE, BLACK);

            GFX_DrawString(31, 39, "Press ENTER", WHITE, BLACK);
        }

        SSD1306_Display(Display);
}
