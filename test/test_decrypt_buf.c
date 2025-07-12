#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <test/test_utils.h>

#include "decrypt.h"

int main(void) {
  
  FILE *efp = fopen("testdata/encrypted.file", "rb");
  if (!efp) {
    return EXIT_FAILURE;
  }

  fseek(efp, 0, SEEK_END);
  size_t enc_len = ftell(efp);
  fseek(efp, 0, SEEK_SET);

  uint8_t *encrypted = malloc(enc_len);
  fread(encrypted, 1, enc_len, efp);
  fclose(efp);

  const char *password = "12345";
  uint8_t *key_hash32 = NULL;

  size_t required_len = 0;

  int rc = fcrypt_decrypt_buf(
    encrypted, 
    enc_len, 
    (uint8_t *)password, 
    strlen((char *)password), 
    NULL, 0, 
    &required_len);

  if (rc != EXIT_SUCCESS) {
    TLOG("rc is not equal to EXIT_SUCCESS");
    return 1;
  }

  if (required_len != 13) {
    TLOG("The required len %zu doesn't match %u", required_len, 13);
    return EXIT_FAILURE;
  }

  uint8_t *decrypted = malloc(required_len);
  size_t out_len = 0;
  rc = fcrypt_decrypt_buf(
    encrypted, 
    enc_len, 
    (uint8_t *)password, 
    strlen((char *)password), 
    decrypted, required_len, 
    &out_len);

  if (rc != EXIT_SUCCESS) {
    TLOG("rc is not equal to EXIT_SUCCESS");
    return 1;
  }

  FILE *ofp = fopen("testdata/origin.txt", "rb");
  if (!ofp) {
    TLOG("can't open file testdata/origin.txt");
    return EXIT_FAILURE;
  }
  
  fseek(ofp, 0, SEEK_END);
  size_t origin_buf_len = ftell(ofp);
  fseek(ofp, 0, SEEK_SET);

  uint8_t *origin = malloc(origin_buf_len);
  fread(origin, 1, origin_buf_len, ofp);
  fclose(ofp);

  if (out_len != origin_buf_len) {
    fprintf(stderr, "decrypted len != origin len\n");
    return 1;
  }

  if (memcmp(decrypted, origin, out_len)) {
    fprintf(stderr, "decrypted bytes != origin bytes\n");
    return 1;
  }

  free(origin);
  free(encrypted);
  free(decrypted);
  free(key_hash32);

  return EXIT_SUCCESS;
}
