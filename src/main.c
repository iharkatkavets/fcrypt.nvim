/* main.c */

#include "decrypt.h"
#include "encrypt.h"
#include "opts_utils.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  options opts = {0};
  if (parse_arguments(&opts, argc, argv) == EXIT_FAILURE) {
    fprintf(stderr, "Error: Wrong parameters\n");
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  if (opts.show_help) {
    print_usage(argv[0]);
    return EXIT_SUCCESS;
  }

  if (opts.show_version) {
    print_version(argv[0]);
    return EXIT_SUCCESS;
  }

  if (opts.decrypt) {
    return fcrypt_decrypt_from_opts(opts);
  }
  else if (opts.encrypt) {
    return fcrypt_encrypt_from_opts(opts);
  }

  print_usage(argv[0]);
  return EXIT_SUCCESS;
}
