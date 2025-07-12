/* decryptor.c */

#include "decrypt.h"
#include "buf_utils.h"
#include "common_utils.h"
#include "xchacha20.h"
#include "convert_utils.h"
#include "core_utils.h"
#include "verbose.h"
#include "opts_utils.h"
#include "log_utils.h"
#include "file_utils.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

int fcrypt_decrypt_buf(
  const uint8_t *in_buf,
  size_t in_len,
  uint8_t *key,
  size_t key_len,
  uint8_t *out_buf, // NULL for sizing
  size_t out_buf_capacity, // 0 for sizing
  size_t *out_len
) {
  uint16_t version = 0;
  uint8_t *hint = NULL;
  uint16_t hint_len = 0;
  uint8_t *key_hash32 = NULL;
  char key_hash_str[32*2+1];
  size_t read = 0;
  XChaCha_ctx ctx;
  uint16_t pad_len = 0;
  uint8_t nonce24[24];
  char nonce24_str[24*2+1];
  uint8_t counter[8] = {0x1};
  uint8_t enc_buf[4096];
  uint8_t dec_buf[4096];
  ssize_t chunk_len = 0;
  char key_hash_str2[32*2+1];
  size_t wrote = 0;

#define CHECK_READ(N) \
    do { \
        if (in_buf && read + (N) > in_len) return EXIT_FAILURE; \
    } while(0)

#define READ_PTR (in_buf ? in_buf + read : NULL)

#define CHECK_WRITE(N) \
    do { \
        if (out_buf && wrote + (N) > out_buf_capacity) return EXIT_FAILURE; \
    } while(0)

#define WRITE_PTR (out_buf ? out_buf + wrote : NULL)

  CHECK_READ(2); read += read_u16_le_buf(READ_PTR, &version);
  DLOG("Format version: %d", version);

  if (version > 0) {
    CHECK_READ(2); read += read_u16_le_buf(READ_PTR, &hint_len);
    DLOG("Hint lenght: %d", hint_len);

    if (hint_len == 0) {
      fprintf(stderr, "No password hint available.\n");
    } else {
      hint = malloc(hint_len);
      if (hint == NULL) {
        LOG_ERR("Failed to allocate memory for hint");
        return EXIT_FAILURE;
      }

      CHECK_READ(hint_len); read += read_mem(READ_PTR, hint, hint_len);
      fprintf(stderr, "Hint: %s\n", hint);
      free(hint);
    }
  }

  key_hash32 = fcrypt_compute_password_hash(key, key_len);
  bytes_to_hexstr(key_hash_str, key_hash32, 32);
  DLOG("SHA256(key): %s\n", key_hash_str);
  VERBOSE("SHA256(key): %s\n", key_hash_str);

  CHECK_READ(24); read += read_mem(READ_PTR, nonce24, 24);
  bytes_to_hexstr(nonce24_str, nonce24, 24);
  DLOG("\nNonce[24]: %s", nonce24_str);
  VERBOSE("\nNonce[24]: %s", nonce24_str);

  xchacha_keysetup(&ctx, key_hash32, nonce24);
  xchacha_set_counter(&ctx, counter);

  CHECK_READ(2); read += read_mem(READ_PTR, enc_buf, 2);

  xchacha_decrypt_bytes(&ctx, enc_buf, (uint8_t*)&pad_len, 2);
  DLOG("Padsize: %u\n", pad_len);
  VERBOSE("Padsize: %u\n", pad_len);

  while (pad_len > 0) {
    chunk_len = MIN(pad_len, sizeof(enc_buf));
    CHECK_READ(chunk_len); read += read_mem(READ_PTR, enc_buf, chunk_len);
    xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, chunk_len);
    pad_len -= chunk_len;
  }

  CHECK_READ(32); read += read_mem(READ_PTR, enc_buf, 32);

  xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, 32);
  bytes_to_hexstr(key_hash_str2, dec_buf, 32);
  DLOG("SHA256(key): %s\n", key_hash_str2);
  if (memcmp(dec_buf, key_hash32, 32) != 0) {
    LOG_ERR("Wrong password.\n");
    return EXIT_FAILURE;
  }

  while(in_len-read) {
    chunk_len = MIN(sizeof(enc_buf), in_len-read);
    DLOG("chunk_len %zu in_len %zu read %zu", chunk_len, in_len, read);
    CHECK_READ(chunk_len); read += read_mem(READ_PTR, enc_buf, chunk_len);
    xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, chunk_len);
    DLOG("before write");
    CHECK_WRITE(chunk_len); wrote += write_mem(WRITE_PTR, dec_buf, chunk_len);
  }

  *out_len = wrote;

  return EXIT_SUCCESS;
} 

int fcrypt_decrypt_payload_fd(int infd, int outfd, const uint8_t *key_hash32) {
  XChaCha_ctx ctx;
  uint16_t padsize = 0;
  uint8_t nonce24[24];
  char nonce24_str[24*2+1];
  uint8_t counter[8] = {0x1};
  uint8_t enc_buf[4096];
  uint8_t dec_buf[4096];
  ssize_t chunk = 0;
  ssize_t read_size = 0;

  if (read(infd, nonce24, 24) != 24) {
    LOG_ERR("Wrong input file. Can't read 24 bytes of nonce.\n");
    return EXIT_FAILURE;
  }

  bytes_to_hexstr(nonce24_str, nonce24, 24);
  DLOG("Nonce[24]: %s\n", nonce24_str);
  VERBOSE("Nonce[24]: %s\n", nonce24_str);

  xchacha_keysetup(&ctx, key_hash32, nonce24);
  xchacha_set_counter(&ctx, counter);

  if (read(infd, enc_buf, 2) != 2) {
    LOG_ERR("Can't read 2 bytes of padsize.\n");
    return EXIT_FAILURE;
  }

  xchacha_decrypt_bytes(&ctx, enc_buf, (uint8_t*)&padsize, 2);
  DLOG("Padsize: %u\n", padsize);

  while (padsize > 0) {
    chunk = MIN(padsize, sizeof(enc_buf));
    read_size = read(infd, enc_buf, chunk);
    if (read_size < 0) {
      LOG_ERR("Can't read file.\n");
      return EXIT_FAILURE;
    }
    if (read_size != chunk) {
      LOG_ERR("Wrong file or password.\n");
      return EXIT_FAILURE;
    }
    xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, chunk);
    padsize -= chunk;
  }

  read_size = read(infd, enc_buf, 32);
  if (read_size < 0) {
    LOG_ERR("Can't read file.\n");
    return EXIT_FAILURE;
  }
  if (read_size != 32) {
    LOG_ERR("Wrong file or password.\n");
    return EXIT_FAILURE;
  }

  xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, 32);
  if (memcmp(dec_buf, key_hash32, 32) != 0) {
    LOG_ERR("Wrong password.\n");
    return EXIT_FAILURE;
  }

  while(true) {
    read_size = read(infd, enc_buf, sizeof(enc_buf));
    if (read_size<0) {
      LOG_ERR("Can't read the input file.\n");
      return EXIT_FAILURE;
    }
    if (!read_size) {
      break;
    }
    xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, read_size);
    if ((write_bytes(outfd, dec_buf, read_size)) != read_size) {
      LOG_ERR("Fail write to output file.\n");
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

int fcrypt_extract_hint_len_fd(int infd, uint16_t *hint_len) {
  uint16_t len = 0;
  if (read_le16(infd, &len) != 0) {
    return EXIT_FAILURE;
  }

  *hint_len = len;
  return EXIT_SUCCESS;
}

int fcrypt_extract_hint_fd(int infd, uint8_t *hint, uint16_t hint_len) {
  if (read_bytes(infd, hint, hint_len) != hint_len) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int fcrypt_extract_hint_buf(const uint8_t *buf, size_t len, uint8_t *hint, uint16_t hint_len) {
  if (hint_len > len) {
    return EXIT_FAILURE;
  }

  memcpy(hint, buf, hint_len);

  return EXIT_SUCCESS;
}

int fcrypt_extract_format_version_fd(int infd, uint16_t *version) {
  if (read_le16(infd, version) != 0) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int fcrypt_decrypt_from_opts(ArgOptions opts) {
  int infd, outfd = STDOUT_FILENO;
  uint8_t key_hash32[32];
  uint16_t version = 0;
  uint8_t *hint;
  uint16_t hint_len;
  char key_hash_str[32*2+1];

  if (opts.verbose) {
    verbose = 1;
  }

  if (opts.output_file && file_exist(opts.output_file)) {
    return EXIT_FAILURE;
  }

  if (create_input_fd(opts.input_file, &infd)) {
    return EXIT_FAILURE;
  }

  if (fcrypt_extract_format_version_fd(infd, &version)) {
    DLOG("Format version: %d", version);
    close(infd);
    return EXIT_FAILURE;
  }

  if (version > 0) {
    if (fcrypt_extract_hint_len_fd(infd, &hint_len)) {
      DLOG("Hint lenght: %d", hint_len);
      close(infd);
      return EXIT_FAILURE;
    }

    hint = malloc(hint_len);
    if (hint == NULL) {
      LOG_ERR("Failed to allocate memory for hint");
      free(hint);
      close(infd);
      return EXIT_FAILURE;
    }
    if (fcrypt_extract_hint_fd(infd, hint, hint_len)) {
      LOG_ERR("Failed to read hint");
      free(hint);
      close(infd);
      return EXIT_FAILURE;
    }
    if (hint_len == 0) {
      fprintf(stderr, "No password hint available.\n");
    } else {
      fprintf(stderr, "Hint: %s\n", hint);
      free(hint);
    }
  }

  if (fcrypt_resolve_decryption_key(key_hash32, opts)) {
    close(infd);
    return EXIT_FAILURE;
  }
  bytes_to_hexstr(key_hash_str, key_hash32, 32);
  DLOG("SHA256(key): %s\n", key_hash_str);
  VERBOSE("SHA256(key): %s\n", key_hash_str);

  if (create_output_fd(opts.output_file, &outfd)) {
    close(infd);
    return EXIT_FAILURE;
  }

  if (fcrypt_decrypt_payload_fd(infd, outfd, key_hash32)) {
    close(infd); close(outfd);
    return EXIT_FAILURE;
  }

  close(infd); close(outfd);

  return 0;
}

