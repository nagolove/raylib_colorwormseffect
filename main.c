#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "worms_effect.h"

int main() {
    int scrw = 1920, scrh = 1080;
    WormsEffect_t e = worms_effect_new();

    InitWindow(scrw, scrh, "color worms");
    SetTargetFPS(30);

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
    }

    UnloadTexture(t);
    worms_effect_free(e);

    CloseWindow();
    return EXIT_SUCCESS;
}
