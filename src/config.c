#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include "tomlc17.h"

enum Type { T_INT, T_DBL, T_BOOL, T_STR, T_COLOR };

typedef struct {
    const char *section, *key;
    enum Type   type;
    void       *dst;
    double      lo, hi;  /* numeric range, inclusive */
} Opt;

static void warn(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fputs("hmatrix: config: ", stderr);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}

static void set_hex(Color *c, int r, int g, int b);

void config_defaults(Config *c) {
    *c = (Config){
        .fps = 30, .transparent_background = true,
        .change_rate = 0.6,
        .bold_head = true, .dim_trail = false,
        .speed_min = 6, .speed_max = 30,
        .length_min = 6, .length_max = 0,
        .density = 1.0,
        .initial_scatter = true, .start_paused = false,
    };
    c->chars[0] = L'0'; c->chars[1] = L'1'; c->chars[2] = 0;
    c->nchars = 2;
    set_hex(&c->head_color,  0x00, 0xff, 0x00);
    set_hex(&c->color,       0x00, 0xaf, 0x00);
    set_hex(&c->trail_color, 0x00, 0x5f, 0x00);
}

/* ---- colors -------------------------------------------------------- */

static int cube_level(int v) {  /* nearest of 0,95,135,175,215,255 */
    static const int lv[6] = {0, 95, 135, 175, 215, 255};
    int best = 0;
    for (int i = 1; i < 6; i++)
        if (abs(v - lv[i]) < abs(v - lv[best])) best = i;
    return best;
}

static void set_hex(Color *c, int r, int g, int b) {
    c->c256 = (short)(16 + 36 * cube_level(r) + 6 * cube_level(g) + cube_level(b));
    /* ANSI bit order: 1=red 2=green 4=blue; threshold 64 keeps dark greens green */
    c->c8 = (short)((r >= 64) | (g >= 64) << 1 | (b >= 64) << 2);
}

/* Palette index 0-255 -> Color; c8 is a rough fallback for small palettes. */
static void set_index(Color *c, int n) {
    c->c256 = (short)n;
    if (n < 8)       c->c8 = (short)n;
    else if (n < 16) c->c8 = (short)(n - 8);
    else if (n < 232) {
        int i = n - 16;
        c->c8 = (short)((i / 36 > 0) | (i / 6 % 6 > 0) << 1 | (i % 6 > 0) << 2);
    } else c->c8 = n >= 240 ? 7 : 0;
}

/* Decodes UTF-8 `s` into `out` (max 256). Returns the count, or -1 if the
 * text is invalid or holds a character that is not exactly one cell wide. */
static int decode_chars(const char *s, wchar_t *out) {
    int n = 0;
    const unsigned char *p = (const unsigned char *)s;
    while (*p) {
        unsigned cp;
        int extra;
        if (*p < 0x80)      { cp = *p;        extra = 0; }
        else if (*p >= 0xF0 && *p < 0xF8) { cp = *p & 0x07; extra = 3; }
        else if (*p >= 0xE0) { cp = *p & 0x0F; extra = 2; }
        else if (*p >= 0xC2) { cp = *p & 0x1F; extra = 1; }
        else return -1;
        p++;
        while (extra--) {
            if ((*p & 0xC0) != 0x80) return -1;
            cp = cp << 6 | (*p++ & 0x3F);
        }
        if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) return -1;
        if (n >= 256 || wcwidth((wchar_t)cp) != 1) return -1;
        out[n++] = (wchar_t)cp;
    }
    return n;
}

static bool parse_color(const char *s, Color *out) {
    static const char *names[8] = {"black", "red", "green", "yellow",
                                   "blue", "magenta", "cyan", "white"};
    for (short i = 0; i < 8; i++)
        if (strcmp(s, names[i]) == 0) { out->c256 = out->c8 = i; return true; }
    unsigned r, g, b;
    char tail;
    if (s[0] == '#' && strlen(s) == 7 &&
        sscanf(s + 1, "%2x%2x%2x%c", &r, &g, &b, &tail) == 3) {
        set_hex(out, (int)r, (int)g, (int)b);
        return true;
    }
    return false;
}

/* ---- option table -------------------------------------------------- */

static Opt *opt_table(Config *c, int *n) {
    static Opt t[18];
    Opt o[] = {
        {"general",    "fps",                    T_INT,   &c->fps, 1, 240},
        {"general",    "transparent_background", T_BOOL,  &c->transparent_background, 0, 0},
        {"characters", "chars",                  T_STR,   c, 0, 256},
        {"characters", "change_rate",            T_DBL,   &c->change_rate, 0, 20},
        {"appearance", "head_color",             T_COLOR, &c->head_color, 0, 0},
        {"appearance", "color",                  T_COLOR, &c->color, 0, 0},
        {"appearance", "trail_color",            T_COLOR, &c->trail_color, 0, 0},
        {"appearance", "bold_head",              T_BOOL,  &c->bold_head, 0, 0},
        {"appearance", "dim_trail",              T_BOOL,  &c->dim_trail, 0, 0},
        {"animation",  "speed_min",              T_DBL,   &c->speed_min, 0.5, 200},
        {"animation",  "speed_max",              T_DBL,   &c->speed_max, 0.5, 200},
        {"animation",  "length_min",             T_INT,   &c->length_min, 1, 1000},
        {"animation",  "length_max",             T_INT,   &c->length_max, 0, 1000},
        {"animation",  "density",                T_DBL,   &c->density, 0.05, 1},
        {"behavior",   "initial_scatter",        T_BOOL,  &c->initial_scatter, 0, 0},
        {"behavior",   "start_paused",           T_BOOL,  &c->start_paused, 0, 0},
    };
    *n = (int)(sizeof o / sizeof *o);
    memcpy(t, o, sizeof o);
    return t;
}

static void apply(const Opt *o, toml_datum_t v) {
    switch (o->type) {
    case T_INT:
        if (v.type != TOML_INT64) { warn("[%s] %s must be an integer; using default", o->section, o->key); return; }
        if (v.u.int64 < (long long)o->lo || v.u.int64 > (long long)o->hi) {
            warn("[%s] %s must be %d-%d; using default", o->section, o->key, (int)o->lo, (int)o->hi);
            return;
        }
        *(int *)o->dst = (int)v.u.int64;
        break;
    case T_DBL: {
        double d;
        if (v.type == TOML_FP64) d = v.u.fp64;
        else if (v.type == TOML_INT64) d = (double)v.u.int64;
        else { warn("[%s] %s must be a number; using default", o->section, o->key); return; }
        if (!(d >= o->lo && d <= o->hi)) {  /* also rejects NaN */
            warn("[%s] %s must be %g-%g; using default", o->section, o->key, o->lo, o->hi);
            return;
        }
        *(double *)o->dst = d;
        break;
    }
    case T_BOOL:
        if (v.type != TOML_BOOLEAN) { warn("[%s] %s must be true or false; using default", o->section, o->key); return; }
        *(bool *)o->dst = v.u.boolean;
        break;
    case T_COLOR:
        if (v.type == TOML_INT64 && v.u.int64 >= 0 && v.u.int64 <= 255)
            set_index(o->dst, (int)v.u.int64);
        else if (v.type != TOML_STRING || !parse_color(v.u.s, o->dst))
            warn("[%s] %s must be a color name, \"#rrggbb\" or a palette index 0-255; using default", o->section, o->key);
        break;
    case T_STR: {
        Config *c = o->dst;
        wchar_t tmp[257];
        int n = v.type == TOML_STRING ? decode_chars(v.u.s, tmp) : -1;
        if (n < 1) {
            warn("[%s] %s must be 1-256 single-width characters (non-ASCII needs a UTF-8 locale); using default", o->section, o->key);
            return;
        }
        memcpy(c->chars, tmp, (size_t)n * sizeof *tmp);
        c->chars[n] = 0;
        c->nchars = n;
        break;
    }
    }
}

static bool known_section(const Opt *t, int n, const char *s) {
    for (int i = 0; i < n; i++) if (!strcmp(t[i].section, s)) return true;
    return false;
}

bool config_load(Config *c, const char *path, bool required) {
    FILE *f = fopen(path, "r");
    if (!f) {
        if (required) fprintf(stderr, "hmatrix: cannot open config '%s'\n", path);
        return !required;
    }
    fclose(f);

    toml_result_t r = toml_parse_file_ex(path);
    if (!r.ok) {
        warn("%s: %s; using defaults", path, r.errmsg);
        toml_free(r);
        return true;
    }

    int n;
    const Opt *t = opt_table(c, &n);

    for (int i = 0; i < r.toptab.u.tab.size; i++) {
        const char *sec = r.toptab.u.tab.key[i];
        toml_datum_t sv = r.toptab.u.tab.value[i];
        if (sv.type != TOML_TABLE || !known_section(t, n, sec)) {
            warn("unknown setting '%s' ignored", sec);
            continue;
        }
        for (int k = 0; k < sv.u.tab.size; k++) {
            const char *key = sv.u.tab.key[k];
            const Opt *o = NULL;
            for (int j = 0; j < n; j++)
                if (!strcmp(t[j].section, sec) && !strcmp(t[j].key, key)) o = &t[j];
            if (o) apply(o, sv.u.tab.value[k]);
            else warn("unknown option '%s.%s' ignored", sec, key);
        }
    }
    toml_free(r);

    /* cross-field checks */
    Config d;
    config_defaults(&d);
    if (c->speed_min > c->speed_max) {
        warn("[animation] speed_min is greater than speed_max; using defaults for both");
        c->speed_min = d.speed_min; c->speed_max = d.speed_max;
    }
    if (c->length_max != 0 && c->length_max < c->length_min) {
        warn("[animation] length_max is smaller than length_min; using defaults for both");
        c->length_min = d.length_min; c->length_max = d.length_max;
    }
    return true;
}

/* First readable file among: ~/.config/hmatrix/hmatrix.toml, then relative to
 * the executable: ../config/hmatrix.toml (source tree) and
 * ../share/hmatrix/hmatrix.toml (installed). */
bool config_find_default(char *buf, int size) {
    const char *home = getenv("HOME");
    if (home && *home &&
        snprintf(buf, (size_t)size, "%s/.config/hmatrix/hmatrix.toml", home) < size &&
        access(buf, R_OK) == 0)
        return true;

    char exe[1024];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof exe - 1);
    if (n <= 0) return false;
    exe[n] = 0;
    char *slash = strrchr(exe, '/');
    if (!slash) return false;
    *slash = 0;
    static const char *rel[] = {"../config/hmatrix.toml", "../share/hmatrix/hmatrix.toml"};
    for (int i = 0; i < 2; i++)
        if (snprintf(buf, (size_t)size, "%s/%s", exe, rel[i]) < size && access(buf, R_OK) == 0)
            return true;
    return false;
}
