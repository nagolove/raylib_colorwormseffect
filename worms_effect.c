#include "worms_effect.h"

#include <assert.h>
#include <stdlib.h>
#include <omp.h>

/*
Original coded by `PetrTurbo` aka turborium
https://github.com/turborium/ColorWormsEffect
*/
 
static const WormsEffectOpts opts_default = {
    .iteration_count = 120,
    .glow_size = 15,
    .swap_direction_rarity = 20,
    .blur_iteration_count = 6400,
    .recolor_rarity = 130,
    .min_value = 10,
    .max_value = 255,
    .glow_min = 10,
    .glow_max = 255,
    .fade_value = {
        .r = 3,
        .g = 4,
        .b = 2,
    },
};

typedef struct Particle {
    int *steps;
    int x, y,
        r, g, b,
        glow_r, glow_g, glow_b;
} Particle;

typedef struct WormsEffect {
    Particle        *particles;
    int             particles_count, step_count;
    Image           bitmap;
    WormsEffectOpts opts;

    void (*fade)(WormsEffect_t e);
    void (*draw)(WormsEffect_t e);
    void (*blur)(WormsEffect_t e);
} WormsEffect;

int RandomRange(int r_start, int r_end) {
    assert(r_start <= r_end);
    return r_start + rand() % (r_end - r_start);
}

void worms_effect_reset(WormsEffect_t e) {
    Particle *particles = e->particles;
    WormsEffectOpts o = e->opts;
    Image *bitmap = &e->bitmap;
    for(int i = 0; i < e->particles_count; i++) {
        Particle *p = &particles[i];
        for(int j = 0; j < e->step_count; j++) {
            p->steps[j] = 0;// ничего
        }
        p->x = RandomRange(0, bitmap->width);
        p->y = RandomRange(0, bitmap->height);
        p->r = RandomRange(o.min_value, o.max_value);
        p->g = RandomRange(o.min_value, o.max_value);
        p->b = RandomRange(o.min_value, o.max_value);
        p->glow_r = RandomRange(o.glow_min, o.glow_max);
        p->glow_g = RandomRange(o.glow_min, o.glow_max);
        p->glow_b = RandomRange(o.glow_min, o.glow_max);
    }
}

int Max(int a, int b) {
    return a > b ? a : b;
}

void fade_mt(WormsEffect_t e) {
    Image *bitmap = &e->bitmap;
    WormsEffectOpts o = e->opts;
    for(int y = 0; y < bitmap->height; y++) {
        for (int x = 0; x < bitmap->width; x++) {
            Color Pixel = GetImageColor(*bitmap, x, y);
            Pixel.r = Max(0, Pixel.r - o.fade_value.r);
            Pixel.g = Max(0, Pixel.g - o.fade_value.g);
            Pixel.b = Max(0, Pixel.b - o.fade_value.b);
            ImageDrawPixel(bitmap, x, y, Pixel);
        }
    }
}

void fade_st(WormsEffect_t e) {
    Image *bitmap = &e->bitmap;
    WormsEffectOpts o = e->opts;
    for(int y = 0; y < bitmap->height; y++) {
        for (int x = 0; x < bitmap->width; x++) {
            Color Pixel = GetImageColor(*bitmap, x, y);
            Pixel.r = Max(0, Pixel.r - o.fade_value.r);
            Pixel.g = Max(0, Pixel.g - o.fade_value.g);
            Pixel.b = Max(0, Pixel.b - o.fade_value.b);
            ImageDrawPixel(bitmap, x, y, Pixel);
        }
    }
}

int Random(int to) {
    return rand() % to;
}

int Min(int a, int b) {
    return a < b ? a : b;
}

// EnsureRange
int e_range(int val, int min, int max) {
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}

void PaintAndUpdateParticle(WormsEffect_t e, Particle *particle) {
    Image *data = &e->bitmap;
    int Index = 0;
    WormsEffectOpts o = e->opts;
    int ParticleGlowSize = o.glow_size,
        ParticleScale = 1;

    for (int i = 0; i < o.iteration_count; i++) {
        // рисуем свечение
        int X = Random(ParticleGlowSize * 2 + 1) - ParticleGlowSize;
        int Y = Random(ParticleGlowSize * 2 + 1) - ParticleGlowSize;

        Color Pixel = GetImageColor(
            *data, 
            particle->x / ParticleScale + X,
            particle->y / ParticleScale + Y
        );

        Pixel.r = Min(255, Pixel.r + particle->glow_r);
        Pixel.g = Min(255, Pixel.g + particle->glow_g);
        Pixel.b = Min(255, Pixel.b + particle->glow_b);

        int x = particle->x / ParticleScale + X;
        int y = particle->y / ParticleScale + Y;
        ImageDrawPixel(data, x, y, Pixel);

        // рисуем точку
        x = particle->x / ParticleScale;
        y = particle->y / ParticleScale;
        Pixel = GetImageColor(*data, x, y);
        Pixel.r = Min(255, Pixel.r + particle->r);
        Pixel.g = Min(255, Pixel.g + particle->g);
        Pixel.b = Min(255, Pixel.b + particle->b);

        x = particle->x / ParticleScale;
        y = particle->y / ParticleScale;
        ImageDrawPixel(data, x, y, Pixel);

        // генерация неправления
        if (Random(o.swap_direction_rarity) != 0) 
            continue;

        particle->steps[Random(e->step_count)] = Random(4);

        // сдвиг
        switch (particle->steps[Index]) {
            case 0: particle->x += 1; particle->y += 1; break;
            case 1: particle->x += -1; particle->y += 1; break;
            case 2: particle->x += 1; particle->y += -1; break;
            case 3: particle->x += -1; particle->y += -1; break;
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
        if (Index >= e->step_count) {
            Index = 0;
        }

        // перекраска
        if (Random(o.recolor_rarity) != 0) 
            continue;

        // Выбираем случайную частицу из массива частиц
        int N = Random(e->particles_count);
        Particle *p = &e->particles[N];
        if (Random(2) == 0) {
            // Изменяем основной цвет частицы (r, g, b)
            switch (Random(3)) {
                case 0: 
                    p->r = e_range(p->r + Random(3) - 1, o.min_value, o.max_value);
                    break;
                case 1: 
                    p->g = e_range(p->g + Random(3) - 1, o.min_value, o.max_value);
                    break;
                case 2: 
                    p->b = e_range(p->b + Random(3) - 1, o.min_value, o.max_value);
                    break;
            }
        } else {
            // Изменяем цвет свечения частицы (glow_r, glow_g, glow_b)
            switch (Random(3)) {
                case 0: 
                    p->glow_r = e_range(p->glow_r + Random(3) - 1, o.glow_min, o.glow_max);
                    break;
                case 1: 
                    p->glow_g = e_range(p->glow_g + Random(3) - 1, o.glow_min, o.glow_max);
                    break;
                case 2: 
                    p->glow_b = e_range(p->glow_b + Random(3) - 1, o.glow_min, o.glow_max);
                    break;
            }
        }

    }
}

void blur_mt(WormsEffect_t e) {
    Image *bitmap = &e->bitmap;

    // Выполняем размытие заданное количество раз
    for (int i = 1; i < e->opts.blur_iteration_count; i++) {
        // Инициализируем суммы цветовых каналов (R, G, B) для вычисления среднего
        int R = 0, G = 0, B = 0,
        // Выбираем случайный пиксель на изображении
            X = Random(bitmap->width),
            Y = Random(bitmap->height);
        Color Pixel = { .a = 128 };

        // Добавляем значения соседних пикселей для вычисления среднего цвета
        if ( X >= 1 ) {
            Pixel = GetImageColor(*bitmap, X - 1, Y);
            R += Pixel.r; G += Pixel.g; B += Pixel.b;
        }
        if ( X < bitmap->width - 1 ) {
            Pixel = GetImageColor(*bitmap, X + 1, Y);
            R += Pixel.r; G += Pixel.g; B += Pixel.b;
        }
        if ( Y >= 1 ) {
            Pixel = GetImageColor(*bitmap, X, Y - 1);
            R += Pixel.r; G += Pixel.g; B += Pixel.b;
        }
        if ( Y < bitmap->height - 1 ) {
            Pixel = GetImageColor(*bitmap, X, Y + 1);
            R += Pixel.r; G += Pixel.g; B += Pixel.b;
        }

        // Рассчитываем среднее значение для каждого цветового канала
        Pixel.r = R / 4;
        Pixel.g = G / 4;
        Pixel.b = B / 4;

        ImageDrawPixel(bitmap, X, Y, Pixel);
    }
}

void blur_st(WormsEffect_t e) {
    Image *bitmap = &e->bitmap;

    // Выполняем размытие заданное количество раз
    for (int i = 1; i < e->opts.blur_iteration_count; i++) {
        // Инициализируем суммы цветовых каналов (R, G, B) для вычисления среднего
        int R = 0, G = 0, B = 0,
        // Выбираем случайный пиксель на изображении
            X = Random(bitmap->width),
            Y = Random(bitmap->height);
        Color Pixel = { .a = 128 };

        // Добавляем значения соседних пикселей для вычисления среднего цвета
        if ( X >= 1 ) {
            Pixel = GetImageColor(*bitmap, X - 1, Y);
            R += Pixel.r; G += Pixel.g; B += Pixel.b;
        }
        if ( X < bitmap->width - 1 ) {
            Pixel = GetImageColor(*bitmap, X + 1, Y);
            R += Pixel.r; G += Pixel.g; B += Pixel.b;
        }
        if ( Y >= 1 ) {
            Pixel = GetImageColor(*bitmap, X, Y - 1);
            R += Pixel.r; G += Pixel.g; B += Pixel.b;
        }
        if ( Y < bitmap->height - 1 ) {
            Pixel = GetImageColor(*bitmap, X, Y + 1);
            R += Pixel.r; G += Pixel.g; B += Pixel.b;
        }

        // Рассчитываем среднее значение для каждого цветового канала
        Pixel.r = R / 4;
        Pixel.g = G / 4;
        Pixel.b = B / 4;

        ImageDrawPixel(bitmap, X, Y, Pixel);
    }
}

static void draw_st(WormsEffect_t e) {
    for (int i = 0; i < e->particles_count; i++) {
        PaintAndUpdateParticle(e, &e->particles[i]);
    }
}

static void draw_mt(WormsEffect_t e) {
/*#pragma omp parallel for*/
    for (int i = 0; i < e->particles_count; i++) {
        PaintAndUpdateParticle(e, &e->particles[i]);
    }
}


WormsEffect_t worms_effect_new(WormsEffectInitOpts init_opts) {
    WormsEffect_t e = calloc(1, sizeof(*e));

    e->particles_count = 100;
    e->step_count = 30;
    e->particles = calloc(e->particles_count, sizeof(e->particles[0]));

    for (int i = 0; i < e->particles_count; i++) {
        size_t sz = sizeof(e->particles[0].steps[0]);
        e->particles[i].steps = calloc(e->step_count, sz);
    }

    e->bitmap = GenImageColor(init_opts.w, init_opts.h, BLACK);
    e->opts = opts_default;

    if (init_opts.multithreads) {
        e->fade = fade_mt;
        e->draw = draw_mt;
        e->blur = blur_mt;
    } else {
        e->fade = fade_st;
        e->draw = draw_st;
        e->blur = blur_st;
    }

    worms_effect_reset(e);
    return e;
}

void worms_effect_draw(WormsEffect_t e) {
    assert(e);
    e->fade(e);
    e->draw(e);
    e->blur(e);
}

void worms_effect_free(WormsEffect_t e) {
    if (!e) 
        return;

    if (e->particles) {

        for (int i = 0; i < e->particles_count; i++) {
            if (e->particles[i].steps)
                free(e->particles[i].steps);
        }

        free(e->particles);
        e->particles_count = 0;
    }

    if (e->bitmap.data) {
        UnloadImage(e->bitmap);
        e->bitmap.data = NULL;
    }

    free(e);
}

WormsEffectOpts worm_effect_options_default() {
    return opts_default;
}

void worms_effect_options_set(WormsEffect_t e, WormsEffectOpts opt) {
    e->opts = opt;
}

WormsEffectOpts worms_effect_options_get(WormsEffect_t e) {
    return e->opts;
}

Image *worms_effect_bitmap(WormsEffect_t e) {
    return &e->bitmap;
}
