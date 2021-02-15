#include <stdio.h>
#include <string.h>
#include "../inc/ast.h"
#include "../inc/lexer.h"
#include "../inc/calc.h"
#include "../inc/parser.h"


int parse_program(char *input) {
  lexer_t lexer;
  lexer_init(&lexer, input, strlen(input));

  int result = parse_start(&lexer);
  if (lexer.err > -1) {
    printf("Syntax error at %d\n", lexer.pos);
    return -1;
  }

  printf("%d\n", result);
  return 0;
}

