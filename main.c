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
    ExpectedFps = 30,
    BitmapDefaultWidth = 240,
    BitmapDefaultHeight = 180,
    DisplayScale = 5,
    ParticleScale = 16,
    ParticleIterationCount = 120,
    ParticleGlowSize = 3,
    ParticleSwapDirectionRarity = 20,
    BlurIterationCount = 2400,
    ParticleRecolorRarity = 70 + 60,
    ParticleMinValue = 1,
    ParticleMaxValue = 9,
    ParticleGlowMinValue = 1,
    ParticleGlowMaxValue = 7,
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
Image bitmap;

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

void fade(Image *data) {
    for(int y = 0; y < data->height; y++) {
        for (int x = 0; x < data->width; x++) {
            Color Pixel = GetImageColor(*data, x, y);
            Pixel.r = Max(0, Pixel.r - FadeRValue);
            Pixel.g = Max(0, Pixel.g - FadeGValue);
            Pixel.b = Max(0, Pixel.b - FadeBValue);
            ImageDrawPixel(data, x, y, Pixel);
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
            particle->steps[Random(ParticleStepCount) - 1] = Random(4);

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

void paint() {
    for (int i = 0; i < ParticleCount; i++) {
        PaintAndUpdateParticle(&bitmap, &particles[i]);
    }
}

void blur() {
    /*
  var
    R, G, B: Integer;
    X, Y: Integer;
    I: Integer;
    Pixel: TPixelRec;
    */
        for (int i = 1; i < BlurIterationCount; i++) {
      int R = 0;
      int G = 0;
      int B = 0;
      int X = Random(bitmap.width);
      int Y = Random(bitmap.height);

      /*
      if X >= 1 then
      begin
        Pixel := Data.GetPixel(X - 1, Y);
        R := R + Pixel.R;
        G := G + Pixel.G;
        B := B + Pixel.B;
      end;
      if X < Data.Width - 1 then
      begin
        Pixel := Data.GetPixel(X + 1, Y);
        R := R + Pixel.R;
        G := G + Pixel.G;
        B := B + Pixel.B;
      end;
      if Y >= 1 then
      begin
        Pixel := Data.GetPixel(X, Y - 1);
        R := R + Pixel.R;
        G := G + Pixel.G;
        B := B + Pixel.B;
      end;
      if Y < Data.Height - 1 then
      begin
        Pixel := Data.GetPixel(X, Y + 1);
        R := R + Pixel.R;
        G := G + Pixel.G;
        B := B + Pixel.B;
      end;
      Pixel.R := R div 4;
      Pixel.G := G div 4;
      Pixel.B := B div 4;
      ImageDrawPixel(bitmap, X, Y, Pixel);
        }
        */
}

int main() {

    int scrw = 1920, scrh = 1080;
    InitWindow(scrw, scrh, "color worms");
    SetTargetFPS(120);
    bitmap = GenImageColor(scrw, scrh, RAYWHITE);

    while (!WindowShouldClose()) {
        ClearBackground(RAYWHITE);                          // Set background color (framebuffer clear color)
        BeginDrawing();                                    // Setup canvas (framebuffer) to start drawing
        EndDrawing();                                      // End canvas drawing and swap buffers (double buffering)
    }

    UnloadImage(bitmap);

    CloseWindow();
    return EXIT_SUCCESS;
}
