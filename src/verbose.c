/* verbose.c */

#include "verbose.h"

#include <stdarg.h>
#include <stdio.h>

int verbose = 0;

void vlog(const char *format, ...) {
    if (verbose) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}
