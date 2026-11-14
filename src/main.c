#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tinyvm.h>

enum {
  EXIT_USAGE = 2,
};

[[noreturn]] static void usage(const char *program) {
  fprintf(stderr, "usage: %s [--version] guest.bin\n", program);
  exit(EXIT_USAGE);
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "--version") == 0) {
    printf("tinyvm %s (chapter 1 scaffold)\n", TINYVM_VERSION);
    return EXIT_SUCCESS;
  }

  if (argc != 2)
    usage(argv[0]);

  fprintf(stderr,
          "tinyvm: guest execution is not implemented yet\n");
  return EXIT_FAILURE;
}
