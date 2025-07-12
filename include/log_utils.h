#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <stdio.h>

#define LOG_ERR(fmt, ...) \
    do { fprintf(stderr, "[%s:%d] " fmt , __FILE__, __LINE__, ##__VA_ARGS__); } while (0)

#define VERBOSE(fmt, ...) \
    do { \
        if (verbose) { \
            fprintf(stderr, fmt , ##__VA_ARGS__); \
        } \
    } while (0)

#ifdef DEBUG
#define DLOG(fmt, ...) \
    do { \
        fprintf(stderr, "[%s:%d] " fmt , __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#else
#define DLOG(fmt, ...) \
    do {} while (0)
#endif

#define DUMP_HEX(label, buf, len)                                      \
    do {                                                               \
        size_t __dump_len = (len);                                     \
        const uint8_t *__dump_buf = (const uint8_t *)(buf);            \
        char __hexstr[__dump_len * 2 + 1];                             \
        bytes_to_hexstr(__hexstr, __dump_buf, __dump_len);            \
        fprintf(stderr, "%s: %s\n", (label), __hexstr);                \
    } while (0)

#endif // LOG_UTILS_H
