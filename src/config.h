#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <wchar.h>

typedef struct {
    short c256;  /* xterm-256 palette index, used when COLORS >= 256 */
    short c8;    /* basic ANSI color 0-7, used on smaller palettes */
} Color;

typedef struct {
    /* [general] */
    int  fps;
    bool transparent_background;
    /* [characters] */
    wchar_t chars[257];  /* custom character set (UTF-8 in the file) */
    int     nchars;
    double change_rate;
    /* [appearance] */
    Color head_color, color, trail_color;
    bool  bold_head, dim_trail;
    /* [animation] */
    double speed_min, speed_max;
    int    length_min, length_max;
    double density;
    /* [behavior] */
    bool initial_scatter, start_paused;
} Config;

void config_defaults(Config *c);

/* Loads `path` over the defaults. Invalid/unknown entries produce a warning
 * on stderr and keep the default. Returns false only if `required` is set
 * and the file cannot be opened. */
bool config_load(Config *c, const char *path, bool required);

/* Finds the default config file (see config.c for the search order).
 * Returns false if none exists, in which case built-in defaults apply. */
bool config_find_default(char *buf, int size);

#endif
