/*
 * GameFlappy.h
 *
 *  Created on: Aug 2026
 *      Author: dryla
 */

#ifndef INC_GAMEFLAPPY_H_
#define INC_GAMEFLAPPY_H_

#include <stdint.h>
#include <stdlib.h>
#include "SSD1306_OLED.h"
#include "GFX_BW.h"

#define FLAPPY_BIRD_SIZE      8
#define FLAPPY_GAP_SIZE       (3 * FLAPPY_BIRD_SIZE) // 24 piksele
#define FLAPPY_PIPE_WIDTH     12                     // Szerokość rury przeszkody
#define FLAPPY_SCREEN_WIDTH   128
#define FLAPPY_SCREEN_HEIGHT  64
#define FLAPPY_BIRD_X         24                     // Sztywna pozycja X ptaka

// Struktura pojedynczej rury (przeszkody)
typedef struct {
    int16_t x;          // Pozycja X rury na ekranie
    int16_t gap_y;      // Pozycja Y, w której zaczyna się luka
    uint8_t Passed;     // Flaga, czy ptak już minął tę rurę (do punktacji)
} FlappyPipe_t;

// Główna struktura gry
typedef struct {
    float BirdY;        // Pozycja Y ptaka (float dla płynnej grawitacji)
    float VelocityY;    // Aktualna prędkość opadania/wznoszenia

    FlappyPipe_t Pipes[2]; // Dwie rury widoczne jednocześnie na ekranie

    uint8_t IsDead;
    uint8_t NeedsRedraw;

    uint32_t LastMoveTime;
    uint32_t SpeedMs;   // Czas odświeżania klatki fizyki

    uint16_t Score;
} GameFlappy_t;

// Globalna instancja gry
extern GameFlappy_t Flappy;

// Funkcje publiczne
void Flappy_Init(void);
void Flappy_UpdateLogic(void);
void Flappy_Draw(SSD1306_t *Display);

// Akcja dla jednego przycisku
void Flappy_Jump(void);

#endif /* INC_GAMEFLAPPY_H_ */