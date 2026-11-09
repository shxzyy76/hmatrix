#ifndef CLI_H
#define CLI_H

typedef struct {
    const char *config_path;  /* NULL = default location */
} CliArgs;

/* Returns -1 to continue running, otherwise the process exit code
 * (0 after --help/--version, 2 on bad usage). */
int cli_parse(int argc, char **argv, CliArgs *out);

#endif
