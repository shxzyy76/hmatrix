#include "renderer.h"

#include <curses.h>

enum { P_HEAD = 1, P_BODY, P_TRAIL, P_BG };

typedef struct {
    attr_t attr;
    short  pair;
} Style;

static Style st_head, st_body, st_trail;

void renderer_init(const Config *cfg) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    set_escdelay(25);

    st_head  = (Style){cfg->bold_head ? A_BOLD : A_NORMAL, 0};
    st_body  = (Style){A_NORMAL, 0};
    st_trail = (Style){cfg->dim_trail ? A_DIM : A_NORMAL, 0};

    if (has_colors() && start_color() == OK && COLORS >= 8) {
        use_default_colors();  /* -1 = the terminal's own background */
        bool big = COLORS >= 256;
        short bg = cfg->transparent_background ? -1 : COLOR_BLACK;
        init_pair(P_HEAD,  big ? cfg->head_color.c256  : cfg->head_color.c8,  bg);
        init_pair(P_BODY,  big ? cfg->color.c256       : cfg->color.c8,       bg);
        init_pair(P_TRAIL, big ? cfg->trail_color.c256 : cfg->trail_color.c8, bg);
        st_head.pair = P_HEAD;
        st_body.pair = P_BODY;
        st_trail.pair = P_TRAIL;
        if (!big) st_trail.attr |= A_DIM;  /* only way to darken on 8/16 colors */
        if (!cfg->transparent_background) {
            init_pair(P_BG, COLOR_BLACK, COLOR_BLACK);
            bkgd(' ' | COLOR_PAIR(P_BG));
        }
    } else {
        st_trail.attr |= A_DIM;  /* no color: brightness is the only cue */
        st_head.attr  |= A_BOLD;
    }
}

void renderer_shutdown(void) {
    endwin();
}

void renderer_size(int *rows, int *cols) {
    getmaxyx(stdscr, *rows, *cols);
}

void renderer_draw(const Animation *a) {
    erase();  /* clears the virtual screen; ncurses sends only the diff */
    for (int c = 0; c < a->cols; c++) {
        const Stream *s = &a->streams[c];
        int head = (int)s->head;
        for (int i = 0; i < s->len; i++) {
            int y = head - i;
            if (y < 0) break;
            if (y >= a->rows) continue;
            const Style *st = i == 0 ? &st_head : i < s->len / 2 ? &st_body : &st_trail;
            wchar_t w[2] = {a->cells[y * a->cols + c], 0};
            cchar_t cc;
            if (setcchar(&cc, w, st->attr, st->pair, NULL) == OK) mvadd_wch(y, c, &cc);
        }
    }
    wnoutrefresh(stdscr);
    doupdate();
}
