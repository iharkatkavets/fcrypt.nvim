#ifndef LOG_UTILS_H
#define LOG_UTILS_H()

#include <stdio.h>

#define LOG_ERR(msg) \
    fprintf(stderr, "[%s:%d] %s\n", __FILE__, __LINE__, msg)

#define LOGF_ERR(fmt, ...) \
    fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, __VA_ARGS__)

#ifdef DEBUG
#define LOG_DBG(fmt, ...) \
    fprintf(stderr, "[DEBUG %s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_DBG(fmt, ...) \
    do {} while (0)
#endif

#endif // LOG_UTILS_H
