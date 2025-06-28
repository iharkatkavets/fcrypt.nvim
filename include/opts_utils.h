/* opts_utils.h */

#ifndef OPTS_UTILS_H
#define OPTS_UTILS_H

#include <getopt.h>
#include <stdlib.h>

typedef struct {
    int encrypt;
    int decrypt;
    char *input_file;
    char *output_file;
    char *password;
    int show_help;
    int show_version;
    int verbose;
    int padsize;
    char *hint;
    int no_hint;
} Options;

typedef Options options;

void print_usage(const char *program_name);
void print_version(const char *program_name);
int parse_arguments(options *opts, int argc, char **argv);

#endif
