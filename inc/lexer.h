#ifndef __LEXER_H
#define __LEXER_H

#include <inttypes.h>
#include <stdbool.h>
#include "token.h"

#define LEXER_BUF_SIZE 128

typedef struct {
  uint32_t pos;
  int err;
  char nextch;
  char buf[LEXER_BUF_SIZE];
  token_t token;
  token_t next_token;
} lexer_t;

void lexer_init(lexer_t *lexer, const char input[], const uint8_t input_size);

void advance(lexer_t *lexer);

bool eat(lexer_t *lexer, tag_t t);

tag_t peek_token_type(lexer_t *lexer);

#endif

