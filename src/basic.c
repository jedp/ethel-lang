#include <stdio.h>
#include <string.h>
#include "lexer.h"

lexer_t lexer;

int main(int argc, char** argv) {
  char *input = "2 + 2";
  lexer_init(&lexer, input, strlen(input));

  for (;;) {
    token_t *token = lexer_next(&lexer);
    printf("token tag %d\n", token->tag);
    if (token->tag == _EOF) {
      break;
    }
  }

  printf("done.\n");
}

