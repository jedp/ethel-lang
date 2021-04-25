#ifndef __LEXER_H
#define __LEXER_H

#include <inttypes.h>
#include "err.h"
#include "def.h"
#include "token.h"

#define LEXER_BUF_SIZE 2048

typedef struct {
  uint32_t pos;
  uint32_t err_pos;
  uint8_t depth; // Gross to have message-passing from the parser here :(
  error_t err;
  char nextch;
  char buf[LEXER_BUF_SIZE];
  token_t token;
  token_t next_token;
} lexer_t;

void lexer_init(lexer_t *lexer, const char input[], const uint32_t input_size);

void advance(lexer_t *lexer);

boolean eat(lexer_t *lexer, tag_t t);

tag_t peek_token_type(lexer_t *lexer);

#endif

