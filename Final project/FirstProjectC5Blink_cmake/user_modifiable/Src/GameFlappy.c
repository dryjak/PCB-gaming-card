/*
 * GameFlappy.c
 *
 *  Created on: Aug 2026
 *      Author: dryla
 */

#include "GameFlappy.h"
#include <stdio.h>
#include "../../main.h"

// Globalna instancja gry
GameFlappy_t Flappy;

// Generowanie rury na podanej pozycji startowej X
static void SpawnPipe(uint8_t index, int16_t startX) {
    Flappy.Pipes[index].x = startX;
    Flappy.Pipes[index].Passed = 0;

    // Ograniczenia luki: min 10px od góry, min 10px od dołu
    int min_y = 10;
    int max_y = FLAPPY_SCREEN_HEIGHT - FLAPPY_GAP_SIZE - 10; 
    
    // Losowanie wysokości, na której zaczyna się luka
    Flappy.Pipes[index].gap_y = min_y + (rand() % (max_y - min_y + 1));
}

void Flappy_Init(void) {
    Flappy.IsDead = 0;
    Flappy.Score = 0;
    Flappy.NeedsRedraw = 1;
    Flappy.SpeedMs = 40; // Częstotliwość odświeżania (ok. 25 FPS dla płynności)
    Flappy.LastMoveTime = HAL_GetTick();

    // Startowa pozycja i fizyka ptaka
    Flappy.BirdY = FLAPPY_SCREEN_HEIGHT / 2.0f;
    Flappy.VelocityY = 0.0f;

    srand(HAL_GetTick());

    // Ustawienie pierwszej rury tuż za krawędzią ekranu
    SpawnPipe(0, FLAPPY_SCREEN_WIDTH + 20);
    // Ustawienie drugiej rury z odstępem 70 pikseli od pierwszej
    SpawnPipe(1, FLAPPY_SCREEN_WIDTH + 20 + 70); 
}

void Flappy_Jump(void) {
    // Reaguj na przycisk tylko, jeśli gra nadal trwa
    if (!Flappy.IsDead) {
        Flappy.VelocityY = -4.0f; // Impuls w górę (wartość ujemna, bo oś Y rośnie w dół)
    }
}

void Flappy_UpdateLogic(void) {
    if (Flappy.IsDead) return;

    if (HAL_GetTick() - Flappy.LastMoveTime < Flappy.SpeedMs) {
        return;
    }
    Flappy.LastMoveTime = HAL_GetTick();
    
    // Ptak i rury się przemieściły, wymuszamy odświeżenie ekranu
    Flappy.NeedsRedraw = 1; 

    // 1. Fizyka ptaka
    Flappy.VelocityY += 0.6f; // Siła grawitacji przyciągająca ptaka w dół
    Flappy.BirdY += Flappy.VelocityY;

    // 2. Sprawdzenie kolizji z ziemią lub sufitem (TUTAJ JEST POPRAWIONA LOGIKA)
    if (Flappy.BirdY < 0) {
        Flappy.BirdY = 0; // Blokujemy ptaka na górnej krawędzi
        Flappy.IsDead = 1;
        return;
    } 
    else if ((Flappy.BirdY + FLAPPY_BIRD_SIZE) >= FLAPPY_SCREEN_HEIGHT) {
        Flappy.BirdY = FLAPPY_SCREEN_HEIGHT - FLAPPY_BIRD_SIZE; // Blokujemy ptaka na dolnej krawędzi
        Flappy.IsDead = 1;
        return;
    }

    // 3. Logika rur i kolizji
    for (int i = 0; i < 2; i++) {
        // Przesuwanie rury w lewo
        Flappy.Pipes[i].x -= 3; 

// Przydzielanie punktu za minięcie rury
        if (!Flappy.Pipes[i].Passed && (Flappy.Pipes[i].x + FLAPPY_PIPE_WIDTH < FLAPPY_BIRD_X)) {
            Flappy.Pipes[i].Passed = 1;
            Flappy.Score++;

            // --- NOWOŚĆ: Przyspieszanie gry ---
            // Zmniejszamy opóźnienie między klatkami (SpeedMs).
            // Ustawiamy bezpieczny limit (np. 15 ms), aby gra nie stała się niemożliwa!
            if (Flappy.SpeedMs > 15) {
                Flappy.SpeedMs -= 1; // Z każdym punktem odejmujemy 1 milisekundę
            }
        }

        // Zwijanie rury: jeśli wyleciała za ekran z lewej, przerzucamy ją na prawą stronę
        if (Flappy.Pipes[i].x < -FLAPPY_PIPE_WIDTH) {
            uint8_t other_pipe = (i == 0) ? 1 : 0;
            SpawnPipe(i, Flappy.Pipes[other_pipe].x + 70); // Utrzymanie stałego dystansu
        }

        // 4. Kolizja z przeszkodami
        // Czy ptak znajduje się na szerokości rury (oś X)?
        if ((FLAPPY_BIRD_X + FLAPPY_BIRD_SIZE) > Flappy.Pipes[i].x && 
            FLAPPY_BIRD_X < (Flappy.Pipes[i].x + FLAPPY_PIPE_WIDTH)) {
            
            // Jeśli tak, to czy znajduje się poza luką (oś Y)?
            if (Flappy.BirdY < Flappy.Pipes[i].gap_y || 
               (Flappy.BirdY + FLAPPY_BIRD_SIZE) > (Flappy.Pipes[i].gap_y + FLAPPY_GAP_SIZE)) {
                Flappy.IsDead = 1;
                return;
            }
        }
    }
}

void Flappy_Draw(SSD1306_t *Display) 
{

    while (HAL_I2C_GetState(Display->I2c) != HAL_I2C_STATE_IDLE)
    {
        __NOP(); 
    }
    SSD1306_Clear(BLACK);

    // 1. Rysowanie rur (Z BEZPIECZNYM OBCINANIEM / CLIPPINGIEM)
    for (int i = 0; i < 2; i++) {
        int16_t draw_x = Flappy.Pipes[i].x;
        int16_t draw_w = FLAPPY_PIPE_WIDTH;

        // A. Obcinanie z lewej strony ekranu (gdy rura wylatuje)
        if (draw_x < 0) {
            draw_w += draw_x; // Zmniejszamy szerokość (draw_x jest ujemne)
            draw_x = 0;       // Zaczynamy rysować równo z lewą krawędzią
        }

        // B. Obcinanie z prawej strony ekranu (gdy rura wlatuje)
        if (draw_x + draw_w > FLAPPY_SCREEN_WIDTH) {
            draw_w = FLAPPY_SCREEN_WIDTH - draw_x; // Skracamy szerokość do krawędzi
        }

        // C. Rysuj tylko, jeśli rura jest chociaż trochę widoczna na ekranie (draw_w > 0)
        if (draw_w > 0 && draw_x < FLAPPY_SCREEN_WIDTH) {
            // Górna rura
            GFX_DrawFillRectangle(
                draw_x, 
                0, 
                draw_w, 
                Flappy.Pipes[i].gap_y, 
                WHITE);
                
            // Dolna rura
            GFX_DrawFillRectangle(
                draw_x, 
                Flappy.Pipes[i].gap_y + FLAPPY_GAP_SIZE, 
                draw_w, 
                FLAPPY_SCREEN_HEIGHT - (Flappy.Pipes[i].gap_y + FLAPPY_GAP_SIZE), 
                WHITE);
        }
    }

    // 2. Rysowanie ptaka (rzutujemy pozycję float na int)
    GFX_DrawFillRectangle(
        FLAPPY_BIRD_X, 
        (int16_t)Flappy.BirdY, 
        FLAPPY_BIRD_SIZE, 
        FLAPPY_BIRD_SIZE, 
        WHITE);

    // 3. Nakładka Game Over
    if (Flappy.IsDead) {
        GFX_DrawString(37, 15, "GAME OVER", WHITE, BLACK);

        char scoreText[20];
        snprintf(scoreText, sizeof(scoreText), "Score: %d", Flappy.Score);
        GFX_DrawString(37, 27 , scoreText, WHITE, BLACK);

        GFX_DrawString(31, 39, "Press ENTER", WHITE, BLACK);
    }

    SSD1306_Display(Display);
}