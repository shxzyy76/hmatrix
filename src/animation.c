#include "animation.h"

#include <stdlib.h>
#include <time.h>

/* xorshift32: cheap, allocation-free randomness */
static unsigned rnd(Animation *a) {
    unsigned x = a->rng;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return a->rng = x;
}

static float rndf(Animation *a) { return (rnd(a) & 0xFFFF) / 65536.0f; }

static wchar_t rnd_char(Animation *a) { return a->cfg->chars[rnd(a) % (unsigned)a->cfg->nchars]; }

static void respawn(Animation *a, Stream *s, bool initial) {
    const Config *c = a->cfg;
    int lmax = c->length_max ? c->length_max : a->rows;
    if (lmax < c->length_min) lmax = c->length_min;
    s->len   = c->length_min + (int)(rnd(a) % (unsigned)(lmax - c->length_min + 1));
    s->speed = (float)c->speed_min + rndf(a) * (float)(c->speed_max - c->speed_min);

    if (initial && c->initial_scatter) s->head = rndf(a) * (float)(a->rows + s->len) - (float)s->len;
    else                               s->head = -rndf(a) * (float)a->rows * 0.5f;

    /* density < 1: idle gap so a column holds a stream ~density of the time */
    if (c->density < 1.0 && (!initial || !c->initial_scatter || rndf(a) > c->density))
        s->head -= (float)(a->rows + s->len) * (float)((1.0 - c->density) / c->density) * 2.0f * rndf(a);
}

void anim_reset(Animation *a) {
    a->rng = (unsigned)time(NULL) | 1u;
    a->flip_acc = 0;
    for (int i = 0; i < a->rows * a->cols; i++) a->cells[i] = rnd_char(a);
    for (int c = 0; c < a->cols; c++) respawn(a, &a->streams[c], true);
}

bool anim_resize(Animation *a, const Config *cfg, int rows, int cols) {
    anim_free(a);
    if (rows < 1 || cols < 1) return false;
    a->streams = malloc(sizeof *a->streams * (size_t)cols);
    a->cells   = malloc((size_t)rows * (size_t)cols * sizeof *a->cells);
    if (!a->streams || !a->cells) { anim_free(a); return false; }
    a->cfg = cfg;
    a->rows = rows;
    a->cols = cols;
    anim_reset(a);
    return true;
}

void anim_update(Animation *a, float dt) {
    for (int c = 0; c < a->cols; c++) {
        Stream *s = &a->streams[c];
        s->head += s->speed * dt;
        if (s->head - (float)s->len > (float)a->rows) respawn(a, s, false);
    }
    unsigned cells = (unsigned)(a->rows * a->cols);
    a->flip_acc += (float)cells * (float)a->cfg->change_rate * dt;
    unsigned flips = (unsigned)a->flip_acc;
    a->flip_acc -= (float)flips;
    if (flips > cells) flips = cells;
    while (flips--) a->cells[rnd(a) % cells] = rnd_char(a);
}

void anim_free(Animation *a) {
    free(a->streams);
    free(a->cells);
    a->streams = NULL;
    a->cells = NULL;
    a->rows = a->cols = 0;
}
