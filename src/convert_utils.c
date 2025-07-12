/* convert_utils.c */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void bytes_to_hexstr(char *out_buf, const uint8_t *in_buf, size_t buf_size) {
  for (size_t i=0; i<buf_size; i++) {
    sprintf(out_buf+(i*2), "%02x", in_buf[i]);
  }
  out_buf[buf_size*2] = '\0';
}
