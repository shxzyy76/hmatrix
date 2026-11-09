#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdbool.h>

#include "config.h"

typedef struct {
    float head;   /* row of leading character; negative = still above screen */
    float speed;  /* rows per second */
    int   len;    /* trail length including head */
} Stream;

typedef struct {
    const Config *cfg;
    int      rows, cols;
    Stream  *streams;  /* one per column */
    wchar_t *cells;    /* rows*cols characters */
    float    flip_acc; /* fractional character changes carried between frames */
    unsigned rng;
} Animation;

/* Allocates for the given size; only called at start and on resize. */
bool anim_resize(Animation *a, const Config *cfg, int rows, int cols);
void anim_reset(Animation *a);
void anim_update(Animation *a, float dt);
void anim_free(Animation *a);

#endif
