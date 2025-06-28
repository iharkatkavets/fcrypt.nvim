/* encrypt.h */

#ifndef FCRYPT_ENCRYPT_H
#define FCRYPT_ENCRYPT_H

#include "opts_utils.h"
#include <stdint.h>

int fcrypt_encrypt_buf(
  const uint8_t *in_buf,
  size_t in_buf_len,
  const uint8_t *key,
  size_t key_len,
  const uint8_t *hint,
  size_t hint_len,
  uint16_t padsize,
  uint8_t *out_buf, // NULL for sizing
  size_t out_buf_capacity, // 0 for sizing
  size_t *out_len
);

int fcrypt_encrypt_from_opts(options opts);

#endif

