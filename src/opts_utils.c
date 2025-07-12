/* opts_utils.c */

#include "opts_utils.h"
#include "version.h"
#include <getopt.h>
#include <stdio.h>

void print_usage(const char *program_name) {
  printf("Usage: %s [-e <FILE> | --encrypt <FILE>] [-d <FILE> | --decrypt <FILE>]\n", program_name);
  printf("       [-p <PASSWORD> | --password <PASSWORD>] [-P <PADSIZE> | --padsize <PADSIZE>]\n");
  printf("       [-i <HINT> | --hint <HINT>] [-n | --nohint] [-o <FILE> | --output <FILE>]\n");
  printf("       [-v | --verbose] [-V] [-h | --help]\n\n");

  printf("Options:\n");
  printf("  -d, --decrypt <FILE>        Run Decrypting file operation.\n");
  printf("  -e, --encrypt <FILE>        Run Encrypting file operation.\n");
  printf("  -p  --password <PASSWORD>   The password.\n");
  printf("  -P, --padsize <value>       Optional. Size of random bytes for padding. Generated randomly in range 0-65535 if not provided.\n");
  printf("  -o, --output <FILE>         Write output to this file.\n");
  printf("  -i  --hint <HINT>           The hint to password. Be careful.\n");
  printf("  -n  --nohint                Do not request or store password hint.\n");
  printf("  -v, --verbose               Enable verbose output.\n");
  printf("  -V                          Display the version number and exit.\n");
  printf("  -h, --help                  Show this help message and exit.\n");
  printf("Examples:\n");
  printf("  %s -e origin.file -o encrypted.file\n", program_name);
  printf("  %s -e origin.file -o encrypted.file -i 'hint' -p 'password'\n", program_name);
  printf("  %s -e origin.file -o encrypted.file -n -p 'password'\n", program_name);
  printf("  %s -e origin.file > encrypted.file\n", program_name);
  printf("  %s -d encrypted.file\n", program_name);
  printf("  %s -d encrypted.file -p 'password'\n", program_name);
  printf("\n");
  printf("Description:\n");
  printf("  A command-line tool for encrypting files using the XChaCha20 algorithm.\n");
  printf("  Provide an input file. Optional parameters\n");
  printf("  include padding size and verbose mode for detailed logs.\n");
}

void print_version(const char *program_name) {
  printf("%s %s\n", program_name, FCRYPT_VERSION);
}

int parse_arguments(options *opts, int argc, char **argv) {
  int option = -1;
  int option_index = 0;
  int padsize = -1;

  struct option long_options[] = {
    {"encrypt", required_argument, 0, 'e'},
    {"padsize", required_argument, 0, 'P'}, 
    {"password",required_argument, 0, 'p'},
    {"decrypt", required_argument, 0, 'd'},
    {"output",  required_argument, 0, 'o'}, 
    {"hint",    required_argument, 0, 'i'}, 
    {"nohint",  required_argument, 0, 'n'}, 
    {"verbose", no_argument,       0, 'v'},
    {"help",    no_argument,       0, 'h'},
    {0,         0,                 0,  0 }
  };

  while ((option = getopt_long(argc, argv, "e:P:p:d:o:i:nhvV", long_options, &option_index)) != -1) {
    switch (option) {
      case 'P':
        padsize = atoi(optarg);
        if (padsize < 0 || padsize > 65535) {
          fprintf(stderr, "Invalid pad size. Must be in range [0, 65535].\n");
          return EXIT_FAILURE;
        }
        break;
      case 'e':
        opts->encrypt = 1;
        opts->input_file = optarg;
        break;
      case 'o':
        opts->output_file = optarg;
        break;
      case 'd':
        opts->decrypt = 1;
        opts->input_file = optarg;
        break;
      case 'p':
        opts->password = optarg;
        break;
      case 'i':
        opts->hint = optarg;
        break;
      case 'n':
        opts->no_hint = 1;
        break;
      case 'v':
        opts->verbose = 1;
        break;
      case 'V':
        opts->show_version = 1;
        return EXIT_SUCCESS;
        break;
      case 'h':
        opts->show_help = 1;
        return EXIT_SUCCESS;
      case '?':
      default:
        return EXIT_FAILURE;
    }
  }

  if (opts->encrypt && opts->decrypt) {
    return EXIT_FAILURE;
  }
  if (opts->no_hint && opts->hint) {
    return EXIT_FAILURE;
  }
  opts->padsize = padsize;

  return EXIT_SUCCESS;
}

ArgOptions default_options(void) {
    return (ArgOptions){
        .encrypt = false,
        .decrypt = false,
        .input_file = NULL,
        .output_file = NULL,
        .password = NULL,
        .padsize = -1,
        .hint = NULL,
        .no_hint = false,
        .show_help = false,
        .show_version = false,
        .verbose = false,
    };
}
