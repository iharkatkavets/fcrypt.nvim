#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "encrypt.h"
#include "test_utils.h"

int main(void) {
  const char *origin_buf = "";
  const uint8_t key[] = "";
  const char *hint = "";
  const size_t pad_len = 0;
  size_t required_len = 0;

  int result = fcrypt_encrypt_buf(
    (uint8_t *)origin_buf, 
    strlen(origin_buf), 
    (uint8_t *)key, 
    strlen((char *)key), 
    (uint8_t *)hint, 
    strlen((char *)hint), 
    pad_len,
    NULL, 0, 
    &required_len);
  if (result != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  if (required_len != 62) {
    TLOG("The required len doesn't match %zu", required_len);  
    return EXIT_FAILURE;
  }

  size_t out_len = 0;
  uint8_t *output = malloc(required_len);
  if (!output) {
    return EXIT_FAILURE;
  }

  result = fcrypt_encrypt_buf(
    (uint8_t *)origin_buf, 
    strlen(origin_buf), 
    (uint8_t *)key, 
    strlen((char *)key), 
    (uint8_t *)hint, 
    strlen((char *)hint), 
    pad_len,
    output, required_len, 
    &out_len);
  if (result != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  free(output);

  return EXIT_SUCCESS;
}
