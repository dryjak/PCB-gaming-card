/*
 * FlappyBird.c
 *
 *  Created on: Jun 29, 2026
 *      Author: dryla
 */

#include "FlappyBird.h"
#include <stdio.h> // Do snprintf

GameFlappy_t Flappy;

// Parametry fizyki grawitacyjnej
const float GRAVITY = 0.45f;       // Siła grawitacji (ciągnie w dół co klatkę)
const float JUMP_STRENGTH = -3.5f; // Siła skoku (ciągnie w górę)
const float PIPE_SPEED = 2.0f;     // Prędkość przesuwania rur w lewo

// Funkcja pomocnicza do losowania wysokości dziury w rurze
static void SpawnPipe(uint8_t index, float start_x) {
    Flappy.Pipes[index].x = start_x;
    // Losujemy przerwę tak, aby nie była za blisko sufitu ani podłogi
    Flappy.Pipes[index].gap_y = 10 + (rand() % 20);
    Flappy.Pipes[index].passed = 0;
}

void Flappy_Init(void) {
    Flappy.BirdY = 32.0f; // Środek ekranu
    Flappy.Velocity = 0.0f;
    Flappy.Score = 0;
    Flappy.IsDead = 0;
    Flappy.NeedsRedraw = 1;
    Flappy.FrameRateMs = 30; // 30 ms na klatkę to super płynna gra

    srand(HAL_GetTick());

    // Inicjalizacja dwóch rur w odpowiedniej odległości
    SpawnPipe(0, 128.0f);                   // Pierwsza rura zaraz za ekranem
    SpawnPipe(1, 128.0f + 70.0f);           // Druga rura 70 pikseli dalej

    Flappy.LastMoveTime = HAL_GetTick();
}

void Flappy_Jump(void) {
    if (!Flappy.IsDead) {
        // Ustawiamy prędkość na stałą ujemną wartość (skok w górę)
        Flappy.Velocity = JUMP_STRENGTH;
    }
}

void Flappy_UpdateLogic(void) {
    if (Flappy.IsDead) return;

    // Non-blocking timer dla stałej prędkości fizyki
    if (HAL_GetTick() - Flappy.LastMoveTime < Flappy.FrameRateMs) {
        return;
    }
    Flappy.LastMoveTime = HAL_GetTick();
    Flappy.NeedsRedraw = 1;

    // 1. FIZYKA PTAKA (Spadanie)
    Flappy.Velocity += GRAVITY;       // Grawitacja przyspiesza ptaka w dół
    Flappy.BirdY += Flappy.Velocity;  // Zmiana pozycji na podstawie prędkości

    // 2. KOLIZJE Z PODŁOGĄ I SUFITEM
    if (Flappy.BirdY < 0.0f || Flappy.BirdY > 63.0f) {
        Flappy.IsDead = 1;
        return;
    }

    // 3. RUCH RUR I KOLIZJE
    for (uint8_t i = 0; i < FLAPPY_MAX_PIPES; i++) {
        Flappy.Pipes[i].x -= PIPE_SPEED; // Rura jedzie w lewo

        // Jeśli rura wyjechała za lewy ekran, respawn z prawej strony
        if (Flappy.Pipes[i].x < -FLAPPY_PIPE_WIDTH) {
            SpawnPipe(i, 128.0f + 10.0f); // Nowa rura po prawej
        }

        // DETEKCJA KOLIZJI (Zderzenie z rurą)
        // Sprawdzamy czy ptak jest w osi X rury
        if (FLAPPY_BIRD_X + FLAPPY_BIRD_SIZE > Flappy.Pipes[i].x &&
            FLAPPY_BIRD_X - FLAPPY_BIRD_SIZE < Flappy.Pipes[i].x + FLAPPY_PIPE_WIDTH) {

            // Ptak jest w osi X rury. Sprawdzamy czy uderzył w rurę (oś Y)
            if (Flappy.BirdY - FLAPPY_BIRD_SIZE < Flappy.Pipes[i].gap_y ||
                Flappy.BirdY + FLAPPY_BIRD_SIZE > Flappy.Pipes[i].gap_y + FLAPPY_PIPE_GAP) {
                Flappy.IsDead = 1;
                return;
            }
        }

        // PUNKTACJA
        // Jeśli środek ptaka minął prawą krawędź rury i flaga to 0
        if (FLAPPY_BIRD_X > Flappy.Pipes[i].x + FLAPPY_PIPE_WIDTH && Flappy.Pipes[i].passed == 0) {
            Flappy.Score++;
            Flappy.Pipes[i].passed = 1;
        }
    }
}

void Flappy_Draw(SSD1306_t *Display) {
    SSD1306_Clear(BLACK);

    // Rysowanie Rur
    for (uint8_t i = 0; i < FLAPPY_MAX_PIPES; i++) {
        // Górna część rury
        GFX_DrawFillRectangle(
            (int)Flappy.Pipes[i].x,
            0,
            FLAPPY_PIPE_WIDTH,
            Flappy.Pipes[i].gap_y,
            WHITE);

        // Dolna część rury
        GFX_DrawFillRectangle(
            (int)Flappy.Pipes[i].x,
            Flappy.Pipes[i].gap_y + FLAPPY_PIPE_GAP,
            FLAPPY_PIPE_WIDTH,
            64 - (Flappy.Pipes[i].gap_y + FLAPPY_PIPE_GAP),
            WHITE);
    }

    // Rysowanie Ptaka (Kółko)
    GFX_DrawFillCircle(FLAPPY_BIRD_X, (int)Flappy.BirdY, FLAPPY_BIRD_SIZE, WHITE);

    // Rysowanie aktualnego wyniku podczas gry (w lewym górnym rogu)
    char scoreText[10];
    snprintf(scoreText, sizeof(scoreText), "%d", Flappy.Score);
    GFX_DrawString(2, 2, scoreText, WHITE, BLACK);

    // Ekran śmierci
    if (Flappy.IsDead) {
        // Rysujemy obramowanie dla czytelności
        GFX_DrawFillRectangle(16, 18, 96, 32, BLACK);
        GFX_DrawRectangle(16, 18, 96, 32, WHITE);

        GFX_DrawString(35, 23, "GAME OVER", WHITE, BLACK);

        char deathScore[20];
        snprintf(deathScore, sizeof(deathScore), "Score: %d", Flappy.Score);
        GFX_DrawString(38, 36, deathScore, WHITE, BLACK);
    }

    SSD1306_Display(Display);
}
