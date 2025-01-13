#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "worms_effect.h"

int main() {
#ifdef _OPENMP
    printf("OpenMP is enabled!\n");
#else
    printf("OpenMP is not enabled.\n");
#endif

    int scrw = 1920, scrh = 1080;
    WormsEffect_t e = worms_effect_new((WormsEffectInitOpts) { 
        .w = scrw,
        .h = scrh,
    });

    InitWindow(scrw, scrh, "color worms");
    SetTargetFPS(60);

    SetTraceLogLevel(LOG_FATAL);

    Texture2D t = LoadTextureFromImage(*worms_effect_bitmap(e));

    while (!WindowShouldClose()) {
        ClearBackground(BLACK); 
        BeginDrawing();

        //paint();
        worms_effect_draw(e);

        void *data = worms_effect_bitmap(e)->data;
        UpdateTexture(t, data);
        DrawTexture(t, 0, 0, WHITE);

        EndDrawing();

        char title[128] = {};
        sprintf(title, "fps %d", GetFPS());
        SetWindowTitle(title);
    }

    UnloadTexture(t);
    worms_effect_free(e);

    CloseWindow();
    return EXIT_SUCCESS;
}
