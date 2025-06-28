/* gen_utils.c */

#include "random.h"
#include "core_utils.h"
#include "log_utils.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/random.h>
#include <termios.h>
#include <stdbool.h>

size_t fcrypt_gen_bytes(uint8_t *buf, const size_t size) {
  uint8_t *p = buf;
  size_t remain = size;

  while (remain) {
    size_t chunk = MIN(remain, (size_t)256);
    if (getentropy(p, chunk)<0) {
      LOG_ERR("Failed to generate random data.\n");
      return remain;
    }
    p += chunk;
    remain -= chunk;
  }

  return remain;
}

size_t fcrypt_gen_uint16(uint16_t *result) {
  uint8_t padsize_buf[2];

  if (fcrypt_gen_bytes(padsize_buf, 2)) {
    return EXIT_FAILURE;
  }

  *result = (uint16_t)((padsize_buf[0] << 8) | padsize_buf[1]);
  return 0;
}

