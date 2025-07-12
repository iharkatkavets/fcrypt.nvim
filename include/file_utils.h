/* file_utils.h */

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

bool file_exist(const char *path);
int write_le16(int outfd, uint16_t value);
int read_le16(int infd, uint16_t *value);
ssize_t write_bytes(int outfd, const uint8_t *buf, ssize_t to_write);
ssize_t read_bytes(int infd, uint8_t *buf, ssize_t count);
int create_input_fd(const char *path, int *infd);
int create_output_fd(const char *path, int *outfd);

#endif

