/* file_utils.c */

#include "file_utils.h"
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

bool file_exist(const char *path) {
  return (access(path, F_OK) == 0);
}

int write_le16(int outfd, uint16_t value) {
  uint8_t buf[2];
  buf[0] = value & 0xFF;        // low byte
  buf[1] = (value >> 8) & 0xFF; // high byte

  return write(outfd, buf, 2) == 2 ? 2 : -1;
}

int read_le16(int infd, uint16_t *value) {
  uint8_t buf[2];
  ssize_t read_len = read(infd, buf, 2);
  if (read_len != 2) 
    return -1;
  *value = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
  return 0;
}

ssize_t write_bytes(int outfd, const uint8_t *buf, ssize_t count) {
  ssize_t chunk_written, total_written = 0;

  while (total_written < count) {
    chunk_written = write(outfd, buf + total_written, count - total_written);
    if (chunk_written < 0) {
      if (errno == EINTR) {
        continue; 
      }
      return -1;
    }
    if (chunk_written == 0) {
      return -1;
    }
    total_written += chunk_written;
  }

  return total_written;
}

ssize_t read_bytes(int infd, uint8_t *buf, ssize_t count) {
  ssize_t read_len = read(infd, buf, count);
  if (read_len != count) 
    return -1;
  return count;
}

int create_input_fd(const char *input_file, int *infd) {
  if ((*infd = open(input_file, O_RDONLY)) < 0) {
    fprintf(stderr, "Can't open '%s' file for reading.\n", input_file);
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}

int create_output_fd(const char *output_file, int *outfd) {
  if (!output_file) {
    *outfd = STDOUT_FILENO;
    return 0;
  }

  if ((*outfd = open(output_file, O_WRONLY|O_CREAT|O_TRUNC, 00600)) == -1) {
    fprintf(stderr, "Can't open '%s' file for writing.\n", output_file);
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}

