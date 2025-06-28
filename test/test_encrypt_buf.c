#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "encrypt.h"

int main(void) {
  FILE *ofp = fopen("testdata/origin.txt", "rb");
  if (!ofp) {
    return EXIT_FAILURE;
  }
  
  fseek(ofp, 0, SEEK_END);
  size_t origin_buf_len = ftell(ofp);
  fseek(ofp, 0, SEEK_SET);

  uint8_t *origin_buf = malloc(origin_buf_len);
  fread(origin_buf, 1, origin_buf_len, ofp);
  fclose(ofp);

  const char *password = "12345";
  const char *hint = "This is the hint";

  size_t required_len;
  int result = fcrypt_encrypt_buf(origin_buf, 
                              origin_buf_len, 
                              (uint8_t *)password, 
                              strlen((char *)password), 
                              (uint8_t *)hint, 
                              strlen((char *)hint), 
                              13,
                              NULL, 0, 
                              &required_len);
  if (result != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  size_t out_len = 0;
  uint8_t *output = malloc(required_len);
  if (!output) {
    return EXIT_FAILURE;
  }
  result = fcrypt_encrypt_buf(origin_buf, 
                              origin_buf_len, 
                              (uint8_t *)password, 
                              strlen((char *)password), 
                              (uint8_t *)hint, 
                              strlen((char *)hint), 
                              13,
                              output, required_len, 
                              &out_len);
  if (result != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  free(output);

  return EXIT_SUCCESS;
}
