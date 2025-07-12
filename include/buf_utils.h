/* buf_utils.h */

#ifndef BUF_UTILS_H
#define BUF_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

static inline size_t write_mem(uint8_t *dst, const void *src, size_t len) {
  if (dst) {
    memcpy(dst, src, len);
  }
  return len;
}

static inline size_t write_u16_le_buf(uint8_t *dst, uint16_t value) {
  if (dst) {
    dst[0] = value & 0xFF;
    dst[1] = value >> 8;
  }
  return 2;
}

static inline size_t read_u16_le_buf(const uint8_t *buf, uint16_t *out) {
    if (!buf || !out) return 0;
    *out = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    return 2;
}

static inline size_t read_mem(const uint8_t *in, uint8_t *out, size_t len) {
  if (out) {
    memcpy(out, in, len);
  }
  return len;
}

static inline size_t read_u16_buf(const uint8_t *buf, uint16_t *out) {
    if (!buf || !out) return 0;
    *out = (uint16_t)buf[1] | ((uint16_t)buf[0] << 8);
    return 2;
}

#endif
