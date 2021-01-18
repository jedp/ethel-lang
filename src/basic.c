#include <stdio.h>
#include <string.h>
#include "parser.h"

#define MAX_INPUT 80

int main(int argc, char** argv) {
  char input[MAX_INPUT];
  memset(input, 0, MAX_INPUT);

  printf("> ");
  if (!fgets(input, sizeof(input), stdin)) {
    printf("No input!");
    return -1;
  }

  return parse_program(input); 
}
