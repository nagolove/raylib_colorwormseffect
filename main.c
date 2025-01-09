#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define ParticleStepCount 70 + 90
#define ParticleCount 34

/*
Original coded by `PetrTurbo` aka turborium
https://github.com/turborium/ColorWormsEffect
*/
 
const int
    ParticleScale = 1,
    ParticleIterationCount = 120,
    ParticleGlowSize = 15,
    ParticleSwapDirectionRarity = 20,
    BlurIterationCount = 3400,
    ParticleRecolorRarity = 70 + 60,
    ParticleMinValue = 10,
    ParticleMaxValue = 255,
    ParticleGlowMinValue = 10,
    ParticleGlowMaxValue = 255,
    FadeRValue = 3,
    FadeGValue = 4,
    FadeBValue = 2;

typedef struct Particle {
    int x, y;
    int steps[ParticleStepCount];
    int ParticleRValue;
    int ParticleGValue;
    int ParticleBValue;
    int ParticleGlowRValue;
    int ParticleGlowGValue;
    int ParticleGlowBValue;
} Particle;

Particle particles[ParticleCount] = {};
Image bitmap = {};

int RandomRange(int r_start, int r_end) {
    assert(r_start <= r_end);
    return r_start + rand() % (r_end - r_start);
}

void reset_effect() {
    for(int i = 0; i < ParticleCount; i++) {
        for(int j = 0; j < ParticleStepCount; j++) {
            particles[i].steps[j] = -1;// ничего
        }
        particles[i].x = bitmap.width * ParticleScale / 2;
        particles[i].y = bitmap.height * ParticleScale / 2;
        particles[i].ParticleRValue = RandomRange(ParticleMinValue, ParticleMaxValue);
        particles[i].ParticleGValue = RandomRange(ParticleMinValue, ParticleMaxValue);
        particles[i].ParticleBValue = RandomRange(ParticleMinValue, ParticleMaxValue);
        particles[i].ParticleGlowRValue = RandomRange(ParticleGlowMinValue, ParticleGlowMaxValue);
        particles[i].ParticleGlowGValue = RandomRange(ParticleGlowMinValue, ParticleGlowMaxValue);
        particles[i].ParticleGlowBValue = RandomRange(ParticleGlowMinValue, ParticleGlowMaxValue);
    }
}

int Max(int a, int b) {
    return a > b ? a : b;
}

void fade() {
    for(int y = 0; y < bitmap.height; y++) {
        for (int x = 0; x < bitmap.width; x++) {
            Color Pixel = GetImageColor(bitmap, x, y);
            Pixel.r = Max(0, Pixel.r - FadeRValue);
            Pixel.g = Max(0, Pixel.g - FadeGValue);
            Pixel.b = Max(0, Pixel.b - FadeBValue);
            ImageDrawPixel(&bitmap, x, y, Pixel);
        }
    }
}

int Random(int to) {
    return rand() % to;
}

int Min(int a, int b) {
    return a < b ? a : b;
}

int EnsureRange(int val, int min, int max) {
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}

void PaintAndUpdateParticle(Image *data, Particle *particle) {
    int Index = 0;
    for (int i = 0; i < ParticleIterationCount; i++) {

        // рисуем свечение
        int X = Random(ParticleGlowSize * 2 + 1) - ParticleGlowSize;
        int Y = Random(ParticleGlowSize * 2 + 1) - ParticleGlowSize;
        Color Pixel = GetImageColor(
                *data, 
                particle->x / ParticleScale + X,
                particle->y / ParticleScale + Y
                );

        Pixel.r = Min(255, Pixel.r + particle->ParticleGlowRValue);
        Pixel.g = Min(255, Pixel.g + particle->ParticleGlowGValue);
        Pixel.b = Min(255, Pixel.b + particle->ParticleGlowBValue);

        ImageDrawPixel(
            data,
            particle->x / ParticleScale + X,
            particle->y / ParticleScale + Y,
            Pixel
        );

        // рисуем точку
        Pixel = GetImageColor(
            *data, particle->x / ParticleScale, particle->y / ParticleScale
        );
        Pixel.r = Min(255, Pixel.r + particle->ParticleRValue);
        Pixel.g = Min(255, Pixel.g + particle->ParticleGValue);
        Pixel.b = Min(255, Pixel.b + particle->ParticleBValue);

        ImageDrawPixel(
                data,
                particle->x / ParticleScale,
                particle->y / ParticleScale,
                Pixel
                );

        // генерация неправления
        if (Random(ParticleSwapDirectionRarity) == 0) {
            particle->steps[Random(ParticleStepCount)] = Random(4);

            // сдвиг
            switch (particle->steps[Index]) {
                case 0:
                    particle->x = particle->x + 1;
                    particle->y = particle->y + 1;
                    break;
                case 1:
                    particle->x = particle->x - 1;
                    particle->y = particle->y + 1;
                    break;
                case 2:
                    particle->x = particle->x + 1;
                    particle->y = particle->y - 1;
                    break;
                case 3:
                    particle->x = particle->x - 1;
                    particle->y = particle->y - 1;
                    break;
            }

            // коррекция
            if ( particle->x < 0 ) {
                particle->x = data->width * ParticleScale - 1;
            };
            if ( particle->x >= data->width * ParticleScale ) {
                particle->x = 0;
            };
            if ( particle->y < 0 ) {
                particle->y = data->height * ParticleScale - 1;
            };
            if ( particle->y >= data->height * ParticleScale ) {
                particle->y = 0;
            };

            // следующий индекс
            Index = Index + 1;
            if (Index >= ParticleStepCount) {
                Index = 0;
            }

            // перекраска
            if (Random(ParticleRecolorRarity) == 0) {
                int N = Random(ParticleCount);
                if (Random(2) == 0) {
                    switch (Random(3)) {
                        case 0: 
                            particles[N].ParticleRValue = EnsureRange(particles[N].ParticleRValue + Random(3) - 1, ParticleMinValue, ParticleMaxValue);
                            break;
                        case 1: 
                            particles[N].ParticleGValue = EnsureRange(particles[N].ParticleGValue + Random(3) - 1, ParticleMinValue, ParticleMaxValue);
                            break;
                        case 2: 
                            particles[N].ParticleBValue = EnsureRange(particles[N].ParticleBValue + Random(3) - 1, ParticleMinValue, ParticleMaxValue);
                            break;
                    }
                } else {
                    switch (Random(3)) {
                        case 0: 
                            particles[N].ParticleGlowRValue = EnsureRange(particles[N].ParticleGlowRValue + Random(3) - 1, ParticleGlowMinValue, ParticleGlowMaxValue);
                            break;
                        case 1: 
                            particles[N].ParticleGlowGValue = EnsureRange(particles[N].ParticleGlowGValue + Random(3) - 1, ParticleGlowMinValue, ParticleGlowMaxValue);
                            break;
                        case 2: 
                            particles[N].ParticleGlowBValue = EnsureRange(particles[N].ParticleGlowBValue + Random(3) - 1, ParticleGlowMinValue, ParticleGlowMaxValue);
                            break;
                    }
                }
            }
        }
    }
}

void blur() {
    for (int i = 1; i < BlurIterationCount; i++) {
        int R = 0;
        int G = 0;
        int B = 0;
        int X = Random(bitmap.width);
        int Y = Random(bitmap.height);
        Color Pixel = { .a = 128 };

        if ( X >= 1 ) {
            Pixel = GetImageColor(bitmap, X - 1, Y);
            R = R + Pixel.r;
            G = G + Pixel.g;
            B = B + Pixel.b;
        }
        if ( X < bitmap.width - 1 ) {
            Pixel = GetImageColor(bitmap, X + 1, Y);
            R = R + Pixel.r;
            G = G + Pixel.g;
            B = B + Pixel.b;
        }
        if ( Y >= 1 ) {
            Pixel = GetImageColor(bitmap, X, Y - 1);
            R = R + Pixel.r;
            G = G + Pixel.g;
            B = B + Pixel.b;
        }
        if ( Y < bitmap.height - 1 ) {
            Pixel = GetImageColor(bitmap, X, Y + 1);
            R = R + Pixel.r;
            G = G + Pixel.g;
            B = B + Pixel.b;
        }

        Pixel.r = R / 4;
        Pixel.g = G / 4;
        Pixel.b = B / 4;

        ImageDrawPixel(&bitmap, X, Y, Pixel);
    }
}

void draw() {
    for (int i = 0; i < ParticleCount; i++) {
        PaintAndUpdateParticle(&bitmap, &particles[i]);
    }
}

void paint() {
    fade();
    draw();
    blur();
}

int main() {
    int scrw = 1920, scrh = 1080;
    InitWindow(scrw, scrh, "color worms");
    SetTargetFPS(30);
    bitmap = GenImageColor(scrw, scrh, BLACK);
    reset_effect();
    SetTraceLogLevel(LOG_FATAL);

    Texture2D t = LoadTextureFromImage(bitmap);

    while (!WindowShouldClose()) {
        ClearBackground(BLACK); 
        BeginDrawing();
        paint();

        UpdateTexture(t, bitmap.data);
        DrawTexture(t, 0, 0, WHITE);

        EndDrawing();
    }

    UnloadImage(bitmap);
    UnloadTexture(t);

    CloseWindow();
    return EXIT_SUCCESS;
}
