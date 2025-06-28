/* input.c */

#include "input.h"
#include <stddef.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

size_t fcrypt_read_password(const char *prompt, uint8_t *out_buf, size_t out_buf_size) {
  size_t read_size;
  struct termios prev_params, new_params;

  if (tcgetattr(fileno(stdin), &prev_params) != 0) {
    fprintf(stderr, "[%s:%d] Fail to read termious state: %s\n", __FILE__, __LINE__, strerror(errno));
    return -1;
  }

  new_params = prev_params;
  new_params.c_lflag &= ~ECHO;

  if (tcsetattr(fileno(stdin), TCSAFLUSH, &new_params) != 0) {
    fprintf(stderr, "[%s:%d] Fail to turn off echo: %s\n", __FILE__, __LINE__, strerror(errno));
    return -1;
  }

  fprintf(stderr, "%s", prompt);
  fflush(stderr);

  read_size = read(STDIN_FILENO, out_buf, out_buf_size);
  if (tcsetattr(fileno(stdin), TCSAFLUSH, &prev_params) != 0) {
    fprintf(stderr, "[%s:%d] Fail to restore terminal settings: %s\n", __FILE__, __LINE__, strerror(errno));
  }

  if (read_size < 1) {
    fprintf(stderr, "[%s:%d] Fail to read input: %s\n", __FILE__, __LINE__, strerror(errno));
    return -1;
  }

  if (read_size == 0) {
    fprintf(stderr, "[%s:%d] No input provided\n", __FILE__, __LINE__);
    return -1;
  }

  if (out_buf[read_size-1] == '\n') {
    read_size--;
  }

  if (read_size < 1) {
    fprintf(stderr, "[%s:%d] No key provided\n", __FILE__, __LINE__);
    return -1;
  }

  return read_size;
}

size_t fcrypt_read_str(const char *prompt, uint8_t *out_buf, size_t out_buf_size) {
  size_t read_size;
  struct termios prev_params, new_params;

  if (tcgetattr(fileno(stdin), &prev_params) != 0) {
    fprintf(stderr, "Fail to read termious state: %s\n", strerror(errno));
    return -1;
  }

  new_params = prev_params;
  new_params.c_lflag |= ECHO;        // enable echo

  if (tcsetattr(fileno(stdin), TCSAFLUSH, &new_params) != 0) {
    fprintf(stderr, "Fail to turn on echo: %s\n", strerror(errno));
    return -1;
  }

  fprintf(stderr, "%s", prompt);
  fflush(stderr);

  read_size = read(STDIN_FILENO, out_buf, out_buf_size);
  if (tcsetattr(fileno(stdin), TCSAFLUSH, &prev_params) != 0) {
    fprintf(stderr, "Failed to restore terminal settings: %s\n", strerror(errno));
  }

  if (read_size < 1) {
    fprintf(stderr, "Failed to read input: %s\n", strerror(errno));
    return -1;
  }

  if (read_size == 0) {
    fprintf(stderr, "No input provided.\n");
    return -1;
  }

  if (out_buf[read_size-1] == '\n') {
    read_size--;
  }

  return read_size;
}
