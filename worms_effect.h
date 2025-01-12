
// TODO: Оформить в виде библиотеки. Дать воможность
// настраивать червяков через imgui

#include <raylib.h>

typedef struct WormsEffectOpts {
    int iteration_count,
        glow_size,
        swap_direction_rarity,
        blur_iteration_count,
        recolor_rarity,
        min_value,
        max_value,
        glow_min,
        glow_max;
    Color fade_value;
} WormsEffectOpts;

typedef struct WormsEffect *WormsEffect_t;

WormsEffectOpts worms_effect_options_default();
void worms_effect_options_set(WormsEffect_t e, WormsEffectOpts opt);
WormsEffectOpts worms_effect_options_get(WormsEffect_t e);
WormsEffect_t worms_effect_new(int w, int h);
void worms_effect_draw(WormsEffect_t e);
void worms_effect_free(WormsEffect_t e);
Image *worms_effect_bitmap(WormsEffect_t e);
