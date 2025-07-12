#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>

#define TLOG(fmt, ...) \
    do { \
        fprintf(stderr, "[%s:%d] " fmt , __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)


#endif // TEST_LOG_UTILS_H
