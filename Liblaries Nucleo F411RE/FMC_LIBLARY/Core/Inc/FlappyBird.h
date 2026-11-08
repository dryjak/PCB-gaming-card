/*
 * FlappyBird.h
 *
 *  Created on: Jun 29, 2026
 *      Author: dryla
 */

#ifndef INC_FLAPPYBIRD_H_
#define INC_FLAPPYBIRD_H_

#include <stdint.h>
#include <stdlib.h>
#include "SSD1306_OLED.h"
#include "GFX_BW.h"

// Ustawienia fizyki i gry
#define FLAPPY_BIRD_X         25   // Pozycja X ptaka (stała)
#define FLAPPY_BIRD_SIZE      3    // Promień ptaka
#define FLAPPY_PIPE_WIDTH     12   // Szerokość rury
#define FLAPPY_PIPE_GAP       24   // Wysokość "dziury" w rurze
#define FLAPPY_MAX_PIPES      2    // Ilość rur na ekranie

typedef struct {
    float x;        // Pozycja rury (używamy float dla płynności)
    int8_t gap_y;   // Górna krawędź dziury (od 10 do 30)
    uint8_t passed; // Flaga, czy ptak już minął tę rurę (do punktacji)
} Pipe_t;

typedef struct {
    float BirdY;        // Pozycja Y ptaka
    float Velocity;     // Prędkość pionowa (dodatnia = spada, ujemna = leci w górę)

    Pipe_t Pipes[FLAPPY_MAX_PIPES]; // Tablica przeszkód

    uint16_t Score;
    uint8_t IsDead;
    uint8_t NeedsRedraw;

    uint32_t LastMoveTime;
    uint32_t FrameRateMs; // Odświeżanie fizyki (np. 30 ms = ~33 FPS)
} GameFlappy_t;

// Zmienna globalna
extern GameFlappy_t Flappy;

// Funkcje publiczne
void Flappy_Init(void);
void Flappy_UpdateLogic(void);
void Flappy_Draw(SSD1306_t *Display);

// Funkcja sterująca (skok)
void Flappy_Jump(void);

#endif /* INC_FLAPPYBIRD_H_ */
