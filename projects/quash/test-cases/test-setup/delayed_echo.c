#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "Incorrect number of arguments\n");
    return EXIT_FAILURE;
  }

  sleep(strtol(argv[2], NULL, 10));
  printf("%s\n", argv[1]);

  return EXIT_SUCCESS;
}
