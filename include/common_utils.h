/* fe.h */

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include "opts_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

bool fcrypt_check_file_exists(const char *path);
int fcrypt_resolve_encryption_key(uint8_t hash_out[32], options opts);
int fcrypt_resolve_decryption_key(uint8_t hash_out[32], options opts);
uint8_t *fcrypt_compute_password_hash(uint8_t *key, size_t keysize);
int fcrypt_resolve_hint(uint8_t **hint_out, size_t *hint_len_out, options opts);
size_t fcrypt_gen_nonce(uint8_t *buf, size_t size);
int fcrypt_gen_pad_size(uint16_t *padsize, options opts);

#endif
