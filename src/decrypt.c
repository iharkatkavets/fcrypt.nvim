/* decryptor.c */

#include "decrypt.h"
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
  uint8_t **out_buf,
  size_t *out_len
) {
  uint16_t version = 0;
  uint8_t *hint = NULL;
  uint16_t hint_len = 0;
  uint8_t *key_hash32 = NULL;
  char key_hash_str[32*2+1];

  if (fcrypt_extract_version_buf(in_buf, in_len, &version)) {
    LOG_ERR("Failed to extract version.\n");
    return EXIT_FAILURE;
  }
  in_buf += 2;
  in_len -= 2;

  if (version > 0) {
    if (fcrypt_extract_hint_len_buf(in_buf, in_len, &hint_len)) {
      LOG_ERR("Failed to extract hint.\n");
      return EXIT_FAILURE;
    }
    in_buf += 2;
    in_len -= 2;
    if (hint_len == 0) {
      fprintf(stderr, "No password hint available.\n");
    } else {
      hint = malloc(hint_len);
      if (hint == NULL) {
        LOG_ERR("Failed to allocate memory for hint.\n");
        return EXIT_FAILURE;
      }
      if (fcrypt_extract_hint_buf(in_buf, in_len, hint, hint_len)) {
        free(hint);
        LOG_ERR("Failed to extract hint.\n");
        return EXIT_FAILURE;
      }
      in_buf += hint_len;
      in_len -= hint_len;
      free(hint);
    }
  }

  key_hash32 = fcrypt_compute_password_hash(key, key_len);
  bytes_to_hexstr(key_hash_str, key_hash32, 32);
  vlog("SHA256(key): %s\n", key_hash_str);

  if (fcrypt_decrypt_payload_buf(in_buf, in_len, key_hash32, out_buf, out_len)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
} 

int fcrypt_decrypt_payload_buf(
    const uint8_t *in_buf,
    size_t in_len,
    const uint8_t *key_hash32,
    uint8_t **out_buf,
    size_t *out_len) {
  XChaCha_ctx ctx;
  uint16_t pad_len = 0;
  uint8_t nonce24[24];
  char nonce24_str[24*2+1];
  uint8_t counter[8] = {0x1};
  uint8_t enc_buf[4096];
  uint8_t dec_buf[4096];
  ssize_t chunk_len = 0;
  ssize_t write_len = 0;

  if (in_len < 24) {
    LOG_ERR("Wrong buffer. Failed to read 24 bytes of nonce.\n");
    return EXIT_FAILURE;
  }
  memcpy(nonce24, in_buf, 24);
  in_len -= 24;
  in_buf += 24;
  bytes_to_hexstr(nonce24_str, nonce24, 24);
  vlog("\nNonce[24]: %s", nonce24_str);

  xchacha_keysetup(&ctx, key_hash32, nonce24);
  xchacha_set_counter(&ctx, counter);

  if (in_len < 2) {
    LOG_ERR("Failed to extract 2 bytes of padsize.\n");
    return EXIT_FAILURE;
  }
  memcpy(enc_buf, in_buf, 2);
  in_len -= 2;
  in_buf += 2;

  xchacha_decrypt_bytes(&ctx, enc_buf, (uint8_t*)&pad_len, 2);
  vlog("Padsize: %u\n", pad_len);

  while (pad_len > 0) {
    chunk_len = MIN(pad_len, sizeof(enc_buf));
    memcpy(enc_buf, in_buf, chunk_len);
    in_len -= chunk_len;
    in_buf += chunk_len;
    xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, chunk_len);
    pad_len -= chunk_len;
  }

  if (in_len < 32) {
    LOG_ERR("Failed to read 32 bytes of encrypted key hash.\n");
    return EXIT_FAILURE;
  }

  memcpy(enc_buf, in_buf, 32);
  in_len -= 32;
  in_buf += 32;

  xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, 32);
  if (memcmp(dec_buf, key_hash32, 32) != 0) {
    LOG_ERR("Wrong password.\n");
    return EXIT_FAILURE;
  }

  *out_buf = malloc(in_len);
  if (*out_buf == NULL) {
    LOG_ERR("Failed to allocate memory for output buffer.\n");
    return EXIT_FAILURE;
  }
  *out_len = in_len;

  while(in_len) {
    chunk_len = MIN(sizeof(enc_buf), in_len);
    memcpy(enc_buf, in_buf, chunk_len);
    in_len -= chunk_len;
    in_buf += chunk_len;
    xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, chunk_len);
    memcpy(*out_buf+write_len, dec_buf, chunk_len);
    write_len += chunk_len;
  }
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
    fprintf(stderr, "Wrong input file. Can't read 24 bytes of nonce.\n");
    return EXIT_FAILURE;
  }

  bytes_to_hexstr(nonce24_str, nonce24, 24);
  vlog("\nNonce[24]: %s", nonce24_str);

  xchacha_keysetup(&ctx, key_hash32, nonce24);
  xchacha_set_counter(&ctx, counter);

  if (read(infd, enc_buf, 2) != 2) {
    fprintf(stderr, "Can't read 2 bytes of padsize.\n");
    return EXIT_FAILURE;
  }

  xchacha_decrypt_bytes(&ctx, enc_buf, (uint8_t*)&padsize, 2);
  vlog("Padsize: %u\n", padsize);

  while (padsize > 0) {
    chunk = MIN(padsize, sizeof(enc_buf));
    read_size = read(infd, enc_buf, chunk);
    if (read_size < 0) {
      fprintf(stderr, "Can't read file.\n");
      return EXIT_FAILURE;
    }
    if (read_size != chunk) {
      fprintf(stderr, "Wrong file or password.\n");
      return EXIT_FAILURE;
    }
    xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, chunk);
    padsize -= chunk;
  }

  read_size = read(infd, enc_buf, 32);
  if (read_size < 0) {
    fprintf(stderr, "Can't read file.\n");
    return EXIT_FAILURE;
  }
  if (read_size != 32) {
    fprintf(stderr, "Wrong file or password.\n");
    return EXIT_FAILURE;
  }

  xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, 32);
  if (memcmp(dec_buf, key_hash32, 32) != 0) {
    fprintf(stderr, "Wrong password.\n");
    return EXIT_FAILURE;
  }

  while(true) {
    read_size = read(infd, enc_buf, sizeof(enc_buf));
    if (read_size<0) {
      fprintf(stderr, "Can't read the input file %d.\n", __LINE__);
      return EXIT_FAILURE;
    }
    if (!read_size) {
      break;
    }
    xchacha_decrypt_bytes(&ctx, enc_buf, dec_buf, read_size);
    if ((write_bytes(outfd, dec_buf, read_size)) != read_size) {
      fprintf(stderr, "Fail write to output file %d.\n", __LINE__);
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

int fcrypt_extract_hint_fd(int infd, uint8_t **hint, uint16_t *hint_len) {
  uint16_t len = 0;
  if (read_le16(infd, &len) != 0) {
    return EXIT_FAILURE;
  }

  *hint = malloc(len);
  if (*hint == NULL) {
    return EXIT_FAILURE;
  }
  if (read_bytes(infd, *hint, len) != len) {
    free(*hint);
    return EXIT_FAILURE;
  }

  *hint_len = len;
  return EXIT_SUCCESS;
}

int fcrypt_extract_hint_len_buf(const uint8_t *buf, size_t len, uint16_t *hint_len) {
  uint16_t hint_len_read = 0;

  if (len < 2) {
    return EXIT_FAILURE;
  }

  hint_len_read = (uint16_t)(buf[0]) | (uint16_t)(buf[1]<<8);
  if ((size_t)hint_len_read > len-2) {
    return EXIT_FAILURE;
  }

  *hint_len = hint_len_read;
  return EXIT_SUCCESS;
}

int fcrypt_extract_hint_buf(const uint8_t *buf, size_t len, uint8_t *hint, uint16_t hint_len) {
  if (hint_len > len) {
    return EXIT_FAILURE;
  }

  memcpy(hint, buf, hint_len);

  return EXIT_SUCCESS;
}

int fcrypt_extract_version_fd(int infd, uint16_t *version) {
  if (read_le16(infd, version) != 0) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int fcrypt_extract_version_buf(const uint8_t *buf, size_t len, uint16_t *version) {
  if (len < 2) {
    return EXIT_FAILURE;
  }
  *version = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
  return EXIT_SUCCESS;
}

int fcrypt_decrypt_from_opts(options opts) {
  int infd, outfd = STDOUT_FILENO;
  uint8_t key_hash32[32];
  uint16_t version = 0;
  uint8_t *hint;
  uint16_t hint_len;
  char key_hash_str[32*2+1];

  if (opts.verbose) {
    verbose = 1;
  }

  if (create_input_fd(opts.input_file, &infd)) {
    return EXIT_FAILURE;
  }

  if (fcrypt_check_file_absent(opts.output_file)) {
    close(infd);
    return EXIT_FAILURE;
  }

  if (fcrypt_extract_version_fd(infd, &version)) {
    close(infd);
    return EXIT_FAILURE;
  }

  if (version > 0) {
    if (fcrypt_extract_hint_fd(infd, &hint, &hint_len)) {
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
  vlog("SHA256(key): %s\n", key_hash_str);

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

