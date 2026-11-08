/*
 * GameSnake.h
 *
 *  Created on: Jun 28, 2026
 *      Author: dryla
 */

#ifndef INC_GAMESNAKE_H_
#define INC_GAMESNAKE_H_

#include <stdint.h>
#include <stdlib.h>
#include "SSD1306_OLED.h"
#include "GFX_BW.h"

#define SNAKE_BLOCK_SIZE  8
#define SNAKE_GRID_WIDTH  (128 / SNAKE_BLOCK_SIZE)
#define SNAKE_GRID_HEIGHT (64 / SNAKE_BLOCK_SIZE)
#define SNAKE_MAX_LENGTH  100 // Max snake length

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} SnakeDirection_t;

// struct for single element of the body
typedef struct {
    int8_t x;
    int8_t y;
} SnakeSegment_t;

// main game structure
typedef struct {
    SnakeSegment_t Body[SNAKE_MAX_LENGTH];
    uint8_t Length;
    SnakeDirection_t Direction;
    SnakeSegment_t Apple;

    uint8_t IsDead;
    uint8_t NeedsRedraw;

    uint32_t LastMoveTime;
    uint32_t SpeedMs;

    uint16_t Score;
} GameSnake_t;

// global variable for main function
extern GameSnake_t Snake;

// Public functions
void Snake_Init(void);
void Snake_UpdateLogic(void);
void Snake_Draw(SSD1306_t *Display);

void Snake_TurnUp(void);
void Snake_TurnDown(void);
void Snake_TurnLeft(void);
void Snake_TurnRight(void);

#endif
