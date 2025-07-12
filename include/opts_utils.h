/* opts_utils.h */

#ifndef OPTS_UTILS_H
#define OPTS_UTILS_H

#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    bool encrypt;
    bool decrypt;
    char *input_file;
    char *output_file;
    char *password;
    bool show_help;
    bool show_version;
    bool verbose;
    int padsize;
    char *hint;
    bool no_hint;
} ArgOptions;

typedef ArgOptions options;

ArgOptions default_options(void);
void print_usage(const char *program_name);
void print_version(const char *program_name);
int parse_arguments(ArgOptions *opts, int argc, char **argv);

#endif
