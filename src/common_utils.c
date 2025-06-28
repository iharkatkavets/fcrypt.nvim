/* main.c */

#include "common_utils.h"

#include "file_utils.h"
#include "input.h"
#include "sha256.h"
#include "convert_utils.h"
#include "random.h"

#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int fcrypt_check_file_absent(const char *output_file) {
  if (output_file && file_exist(output_file)) {
    fprintf(stderr, "Output file '%s' exists.\n", output_file);
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}

int fcrypt_resolve_encryption_key(uint8_t hash_out[32], options opts) {
  uint8_t *key1;
  size_t keysize1;

  if (opts.password) {
    keysize1 = strlen(opts.password);
    key1 = malloc(keysize1);
    memcpy(key1, opts.password, keysize1);
  } else {
    key1 = malloc(256);
    keysize1 = fcrypt_read_password("Enter password: ", key1, 256);
    if (!keysize1) {
      free(key1);
      fprintf(stderr, "\nAbort\n");
      return EXIT_FAILURE;
    } 
    uint8_t *key2 = malloc(256);
    size_t keysize2 = fcrypt_read_password("\nVerify password: ", key2, 256);
    if (!keysize2) {
      free(key1); free(key2);
      fprintf(stderr, "\nAbort\n");
      return EXIT_FAILURE;
    } 
    if (keysize1 != keysize2 || memcmp(key1, key2, keysize1)) {
      free(key1); free(key2);
      fprintf(stderr, "\nError: Passwords do not match.");
      return EXIT_FAILURE;
    }
    free(key2);
  }

  uint8_t *hash = fcrypt_compute_password_hash(key1, keysize1);
  memcpy(hash_out, hash, 32);
  memset(key1, 0, keysize1);
  free(key1);

  return EXIT_SUCCESS;
}


int fcrypt_resolve_decryption_key(uint8_t hash_out[32], options opts) {
  uint8_t *key1 = NULL;
  size_t keysize1 = 0;

  if (opts.password) {
    keysize1 = strlen(opts.password);
    key1 = malloc(keysize1);
    memcpy(key1, opts.password, keysize1);
  } else {
    key1 = malloc(256);
    keysize1 = fcrypt_read_password("Enter password: ", key1, 256);
    if (!keysize1) {
      free(key1);
      fprintf(stderr, "\nAbort\n");
      return EXIT_FAILURE;
    }
    fprintf(stderr, "\n");
  }

  uint8_t *hash = fcrypt_compute_password_hash(key1, keysize1);
  memcpy(hash_out, hash, 32);
  memset(key1, 0, keysize1);
  free(key1);

  return EXIT_SUCCESS;
}


uint8_t *fcrypt_compute_password_hash(uint8_t *key, size_t keysize) {
  uint8_t *hash = sha256_data(key, keysize);
  return hash;
}


int fcrypt_resolve_hint(uint8_t **hint_out, size_t *hint_len_out, options opts) {
  if (opts.hint) {
    *hint_len_out = strlen(opts.hint);
    *hint_out = malloc(*hint_len_out);
    if (!*hint_out) return EXIT_FAILURE;
    memcpy(*hint_out, opts.hint, *hint_len_out);
  } else {
    size_t hintsize;
    uint8_t *hint = malloc(256);
    if (!hint) return EXIT_FAILURE;

    hintsize = fcrypt_read_str("\nEnter password hint: ", hint, 256);
    if (!hintsize) {
      *hint_out = NULL;
      *hint_len_out = 0;
      free(hint);
      return EXIT_SUCCESS;
    }

    *hint_len_out = hintsize;
    *hint_out = malloc(hintsize);
    if (!*hint_out) {
      free(hint);
      return EXIT_FAILURE;
    }

    memcpy(*hint_out, hint, hintsize);
    free(hint);
  }

  return EXIT_SUCCESS;
}

size_t fcrypt_gen_nonce(uint8_t *buf, size_t size) {
  char nonce24_str[size*2+1];
  size_t result; 

  if (!(result = fcrypt_gen_bytes(buf, size))) {
  //   bytes_to_hexstr(nonce24_str, buf, size);
  //   vlog("Nonce[24]: %s\n", nonce24_str);
  }
  return result;
}


int fcrypt_gen_pad_size(uint16_t *padsize, options opts) {
  if (opts.padsize != -1) {
    *padsize = (uint16_t)opts.padsize;
    return EXIT_SUCCESS;
  }

  if(fcrypt_gen_uint16(padsize)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

