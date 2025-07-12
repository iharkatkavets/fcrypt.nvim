/* encryptor.c */

#include "encrypt.h"
#include "buf_utils.h"
#include "common_utils.h"
#include "log_utils.h"
#include "version.h"
#include "xchacha20.h"
#include "convert_utils.h"
#include "core_utils.h"
#include "verbose.h"
#include "opts_utils.h"
#include "random.h"
#include "version.h"
#include "file_utils.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>


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
) {
  char key_hash_str[32*2+1];
  uint8_t *key_hash32;
  uint8_t nonce24[24];
  char nonce24_str[24*2+1];
  XChaCha_ctx ctx;
  uint8_t counter[8] = {0x1};
  uint8_t enc_buf[4096];
  size_t chunk = 0;
  uint8_t pad_buf[4096];
  size_t wrote = 0;

  if (fcrypt_gen_nonce(nonce24, 24)) {
    LOG_ERR("Can't generate IV.\n");
    return EXIT_FAILURE;
  }
  bytes_to_hexstr(nonce24_str, nonce24, 24);
  VERBOSE("Nonce[24]: %s\n", nonce24_str);

#define CHECK_WRITE(N) \
    do { \
        if (out_buf && wrote + (N) > out_buf_capacity) return EXIT_FAILURE; \
    } while(0)

#define WRITE_PTR (out_buf ? out_buf + wrote : NULL)

  CHECK_WRITE(2); wrote += write_mem(WRITE_PTR, FORMAT_VERSION, 2);
  CHECK_WRITE(2); wrote += write_u16_le_buf(WRITE_PTR, hint_len);
  CHECK_WRITE(hint_len); wrote += write_mem(WRITE_PTR, hint, hint_len);
  CHECK_WRITE(24); wrote += write_mem(WRITE_PTR, nonce24, 24);

  key_hash32 = fcrypt_compute_password_hash(key, key_len);
  bytes_to_hexstr(key_hash_str, key_hash32, 32);
  VERBOSE("SHA256(key): %s\n", key_hash_str);

  xchacha_keysetup(&ctx, key_hash32, nonce24);
  xchacha_set_counter(&ctx, counter);

  xchacha_encrypt_bytes(&ctx, (uint8_t*)&(padsize), enc_buf, 2);
  CHECK_WRITE(2); wrote += write_mem(WRITE_PTR, enc_buf, 2);

  uint16_t pad_left = padsize;
  while (pad_left > 0) {
    chunk = MIN((size_t)pad_left, sizeof(enc_buf));
    if (fcrypt_gen_bytes(pad_buf, chunk)) {
      LOG_ERR("Failed to generate secure bytes.\n");
      return EXIT_FAILURE;
    }
    xchacha_encrypt_bytes(&ctx, pad_buf, enc_buf, chunk);
    CHECK_WRITE(chunk); wrote += write_mem(WRITE_PTR, enc_buf, chunk);
    pad_left -= chunk;
  }

  xchacha_encrypt_bytes(&ctx, key_hash32, enc_buf, 32);
  CHECK_WRITE(32); wrote += write_mem(WRITE_PTR, enc_buf, 32);

  size_t in_buf_offset = 0;
  while(in_buf_offset < in_buf_len) {
    chunk = MIN(in_buf_len-in_buf_offset, sizeof(enc_buf));
    xchacha_encrypt_bytes(&ctx, in_buf+in_buf_offset, enc_buf, chunk);
    CHECK_WRITE(chunk); wrote += write_mem(WRITE_PTR, enc_buf, chunk);
    in_buf_offset += chunk;
  }

  *out_len = wrote;

  return EXIT_SUCCESS;
} 

 
int fcrypt_encrypt_file(
  options opts, 
  int infd, 
  int outfd, 
  uint8_t *key_hash32, 
  ssize_t hint_len, 
  uint8_t *hint_buf
) {
  XChaCha_ctx ctx;
  uint8_t counter[8] = {0x1};
  uint8_t enc_buf[4096];
  ssize_t chunk = 0;
  uint8_t dec_buf[4096];
  ssize_t read_size = 0;
  uint8_t pad_buf[4096];
  uint8_t nonce24[24];
  uint16_t padsize = -1;

  if (fcrypt_gen_nonce(nonce24, 24)) {
    LOG_ERR("Can't generate IV.\n");
    return EXIT_FAILURE;
  }

  if ((write_bytes(outfd, FORMAT_VERSION, 2)) != 2) {
    LOG_ERR("Fail to write to file.\n");
    return EXIT_FAILURE;
  }

  if ((write_le16(outfd, (uint16_t)hint_len)) != 2) {
    LOG_ERR("Fail to write to file.\n");
    return EXIT_FAILURE;
  }

  if ((write_bytes(outfd, hint_buf, hint_len)) != hint_len) {
    LOG_ERR("Fail to write to file.\n");
    return EXIT_FAILURE;
  }

  if ((write_bytes(outfd, nonce24, 24)) != 24) {
    LOG_ERR("Fail to write to file.\n");
    return EXIT_FAILURE;
  }

  if (fcrypt_gen_pad_size(&padsize, opts)) {
    LOG_ERR("Fail to generate pad size.\n");
    return EXIT_FAILURE;
  }
  VERBOSE("Padsize: %u\n", padsize);

  xchacha_keysetup(&ctx, key_hash32, nonce24);
  xchacha_set_counter(&ctx, counter);

  xchacha_encrypt_bytes(&ctx, (uint8_t*)&(padsize), enc_buf, 2);
  if ((write_bytes(outfd, enc_buf, 2)) != 2) {
    LOG_ERR("Failed to write to file.\n");
    return EXIT_FAILURE;
  }

  while (padsize > 0) {
    chunk = MIN((size_t)padsize, sizeof(enc_buf));
    if (fcrypt_gen_bytes(pad_buf, chunk)) {
      LOG_ERR("Failed to generate secure bytes.\n");
      return EXIT_FAILURE;
    }
    xchacha_encrypt_bytes(&ctx, pad_buf, enc_buf, chunk);
    if ((write_bytes(outfd, enc_buf, chunk)) != chunk) {
      LOG_ERR("Failed to write to file.\n");
      return EXIT_FAILURE;
    }
    padsize -= chunk;
  }

  xchacha_encrypt_bytes(&ctx, key_hash32, enc_buf, 32);
  if ((write_bytes(outfd, enc_buf, 32)) != 32) {
    LOG_ERR("Failed to write to file.\n");
    return EXIT_FAILURE;
  }

  while(true) {
    read_size = read(infd, dec_buf, sizeof(dec_buf));
    if (read_size < 0) {
      LOG_ERR("Failed read input file.\n");
      return EXIT_FAILURE;
    }
    if (!read_size) {
      break;
    }
    xchacha_encrypt_bytes(&ctx, dec_buf, enc_buf, read_size);
    if ((write_bytes(outfd, enc_buf, read_size)) != read_size) {
      LOG_ERR("Failed to write to file.\n");
      return EXIT_FAILURE;
    }
  }
  return 0;
}


int fcrypt_encrypt_from_opts(options opts) {
  int infd, outfd;
  uint8_t key_hash32[32];
  uint8_t *hint;
  size_t hint_len = 0;

  if (opts.verbose) {
    verbose = 1;
  }

  if (create_input_fd(opts.input_file, &infd)) {
    return EXIT_FAILURE;
  }

  if (opts.output_file && file_exist(opts.output_file)) {
    close(infd);
    return EXIT_FAILURE;
  }

  if (fcrypt_resolve_encryption_key(key_hash32, opts)) {
    close(infd);
    return EXIT_FAILURE;
  }

  if (!opts.no_hint) {
    if (fcrypt_resolve_hint(&hint, &hint_len, opts)) {
      close(infd);
      return EXIT_FAILURE;
    }
  }

  if (create_output_fd(opts.output_file, &outfd)) {
    free(hint);
    close(infd);
    return EXIT_FAILURE;
  }

  if (fcrypt_encrypt_file(opts, infd, outfd, key_hash32, hint_len, hint)) {
    close(infd); close(outfd);
    return EXIT_FAILURE;
  }

  close(infd); close(outfd);

  return EXIT_SUCCESS;
}
