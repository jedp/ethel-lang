#include <stdio.h>
#include <string.h>
#include "parser.h"

#define MAX_INPUT 80

int main(int argc, char** argv) {
  char input[MAX_INPUT];
  memset(input, 0, MAX_INPUT);

  while (1) {
    printf("> ");
    if (!fgets(input, sizeof(input), stdin)) {
      printf("Bye!\n");
      return -1;
    }
    parse_program(input);
  }

  return 0;
}
