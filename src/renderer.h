#ifndef RENDERER_H
#define RENDERER_H

#include "animation.h"
#include "config.h"

void renderer_init(const Config *cfg);
void renderer_shutdown(void);
void renderer_size(int *rows, int *cols);
void renderer_draw(const Animation *a);

#endif
