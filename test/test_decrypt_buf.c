#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

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

  uint8_t *output = NULL;
  size_t out_len = 0;

  int rc = fcrypt_decrypt_buf(
    encrypted, 
    enc_len, 
    (uint8_t *)password, 
    strlen((char *)password), 
    &output, 
    &out_len);

  if (rc != EXIT_SUCCESS) {
    return 1;
  }

  FILE *ofp = fopen("testdata/origin.txt", "rb");
  if (!ofp) {
    return EXIT_FAILURE;
  }
  
  fseek(ofp, 0, SEEK_END);
  size_t or_len = ftell(ofp);
  fseek(ofp, 0, SEEK_SET);

  uint8_t *origin = malloc(or_len);
  fread(origin, 1, or_len, ofp);
  fclose(ofp);

  if (out_len != or_len) {
    fprintf(stderr, "decrypted len != origin len\n");
    return 1;
  }

  if (memcmp(output, origin, out_len)) {
    fprintf(stderr, "decrypted bytes != origin bytes\n");
    return 1;
  }

  free(origin);
  free(encrypted);
  free(output);
  free(key_hash32);

  return 0;
}
