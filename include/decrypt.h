/* decrypt.h */

#ifndef FCRYPT_DECRYPT_H
#define FCRYPT_DECRYPT_H

#include <stdint.h>

#include "opts_utils.h"

int fcrypt_decrypt_buf(
  const uint8_t *in_buf,
  size_t in_len,
  uint8_t *key,
  size_t key_len,
  uint8_t **out_buf,
  size_t *out_len
);

int fcrypt_decrypt_payload_buf(
    const uint8_t *in_buf,
    size_t in_len,
    const uint8_t *key_hash32,
    uint8_t **out_buf,
    size_t *out_len
);

int fcrypt_decrypt_payload_fd(int infd, int outfd, const uint8_t *key_hash32);
int fcrypt_extract_hint_fd(int infd, uint8_t **hint, uint16_t *hint_len);
int fcrypt_extract_hint_len_buf(const uint8_t *buf, size_t len, uint16_t *hint_len);
int fcrypt_extract_hint_buf(const uint8_t *buf, size_t len, uint8_t *hint, uint16_t hint_len);
int fcrypt_extract_version_fd(int infd, uint16_t *version);
int fcrypt_extract_version_buf(const uint8_t *buf, size_t len, uint16_t *version);
int fcrypt_decrypt_from_opts(options opts);

#endif

