#ifndef __LEXER_H
#define __LEXER_H

#include <inttypes.h>
#include <stdbool.h>

#define LEXER_BUF_SIZE 128

typedef enum {
  TAG_EOF = 0,
  TAG_IDENT,
  TAG_VARNAME,
  TAG_LPAREN,
  TAG_RPAREN,
  TAG_PLUS,
  TAG_MINUS,
  TAG_TIMES,
  TAG_DIVIDE,
  TAG_INT,
  TAG_FLOAT,
  TAG_CHAR,
  TAG_STRING,
  TAG_ASSIGN,
  TAG_IF,
  TAG_AND,
  TAG_OR,
  TAG_GT,
  TAG_LT,
  TAG_GE,
  TAG_LE,
  TAG_EQ,
  TAG_FOR,
  TAG_IN,
  TAG_TO,
  TAG_STEP,
  TAG_ABS,
  TAG_SIN,
  TAG_COS,
  TAG_TAN,
  TAG_SQRT,
  TAG_EXP,
  TAG_LN,
  TAG_LOG,
} tag_t;

typedef struct {
  tag_t tag;
  union {
    int32_t intval;
    float floatval;
    char* string;
  } ;
} token_t;

static const token_t reserved[] = {
  { TAG_IF,      .string = (char *) "if" },
  { TAG_AND,     .string = (char *) "and" },
  { TAG_OR,      .string = (char *) "or" },
  { TAG_FOR,     .string = (char *) "for" },
  { TAG_IN,      .string = (char *) "in" },
  { TAG_TO,      .string = (char *) "to" },
  { TAG_STEP,    .string = (char *) "step" },
  { TAG_ABS,     .string = (char *) "abs" },
  { TAG_SIN,     .string = (char *) "sin" },
  { TAG_COS,     .string = (char *) "cos" },
  { TAG_TAN,     .string = (char *) "tan" },
  { TAG_SQRT,    .string = (char *) "sqrt" },
  { TAG_EXP,     .string = (char *) "exp" },
  { TAG_LN,      .string = (char *) "ln" },
  { TAG_LOG,     .string = (char *) "log" },
};

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

