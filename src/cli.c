#include "cli.h"

#include <stdio.h>
#include <string.h>

static void usage(FILE *o) {
    fputs("Usage: hmatrix [OPTIONS]\n\n"
          "Binary digital-rain terminal animation.\n\n"
          "Options:\n"
          "  --config PATH   Read configuration from PATH\n"
          "                  (default: ~/.config/hmatrix/hmatrix.toml, else the\n"
          "                  config/ folder next to the executable)\n"
          "  --help          Show this help and exit\n"
          "  --version       Show the version and exit\n\n"
          "Controls:\n"
          "  Q / Esc  quit      P  pause/resume      R  restart\n", o);
}

int cli_parse(int argc, char **argv, CliArgs *out) {
    out->config_path = NULL;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            usage(stdout);
            return 0;
        } else if (!strcmp(argv[i], "--version")) {
            puts("hmatrix " HMATRIX_VERSION);
            return 0;
        } else if (!strcmp(argv[i], "--config")) {
            if (++i >= argc) { fputs("hmatrix: --config needs a PATH\n", stderr); return 2; }
            out->config_path = argv[i];
        } else if (!strncmp(argv[i], "--config=", 9)) {
            out->config_path = argv[i] + 9;
        } else {
            fprintf(stderr, "hmatrix: unknown option '%s'\n\n", argv[i]);
            usage(stderr);
            return 2;
        }
    }
    return -1;
}
