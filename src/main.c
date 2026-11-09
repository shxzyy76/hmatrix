#include <curses.h>
#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "animation.h"
#include "cli.h"
#include "config.h"
#include "renderer.h"

static volatile sig_atomic_t stop_flag;

static void on_signal(int sig) { (void)sig; stop_flag = 1; }

static double now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

int main(int argc, char **argv) {
    setlocale(LC_ALL, "");  /* needed for UTF-8 output and character widths */
    CliArgs args;
    int rc = cli_parse(argc, argv, &args);
    if (rc >= 0) return rc;

    Config cfg;
    config_defaults(&cfg);
    if (args.config_path) {
        if (!config_load(&cfg, args.config_path, true)) return 1;
    } else {
        char path[1024];
        if (config_find_default(path, sizeof path)) config_load(&cfg, path, false);
    }

    struct sigaction sa = {.sa_handler = on_signal};
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    Animation a = {0};
    int rows, cols;

    renderer_init(&cfg);
    renderer_size(&rows, &cols);
    /* a failed resize leaves a.cells NULL: the loop then idles until resized or quit */
    anim_resize(&a, &cfg, rows, cols);

    bool paused = cfg.start_paused, running = true;
    const double frame = 1.0 / cfg.fps;
    double last = now();

    while (running && !stop_flag) {
        double start = now();
        int ch;
        while ((ch = getch()) != ERR) {
            switch (ch) {
            case 'q': case 'Q': case 27: running = false; break;
            case 'p': case 'P': paused = !paused; break;
            case 'r': case 'R': if (a.cells) anim_reset(&a); break;
            case KEY_RESIZE:
                renderer_size(&rows, &cols);
                anim_resize(&a, &cfg, rows, cols);
                break;
            }
        }

        float dt = (float)(start - last);
        last = start;
        if (dt > 0.1f) dt = 0.1f;  /* avoid a jump after suspend */
        if (a.cells) {
            if (!paused) anim_update(&a, dt);
            renderer_draw(&a);
        }

        double left = frame - (now() - start);
        if (left > 0) {
            struct timespec ts = {(time_t)left, (long)((left - (time_t)left) * 1e9)};
            nanosleep(&ts, NULL);
        }
    }

    anim_free(&a);
    renderer_shutdown();
    return 0;
}
