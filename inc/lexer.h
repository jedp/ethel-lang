#ifndef __LEXER_H
#define __LEXER_H

#include <inttypes.h>
#include <stdbool.h>

#define LEXER_BUF_SIZE 128

typedef enum {
  _EOF = 0,
  LIST,
  PRINT,
  INPUT,
  DATA,
  READ,
  RESTORE,
  IF,
  THEN,
  ELSE,
  EQ,
  NE,
  GT,
  LT,
  GE,
  LE,
  AND,
  OR,
  TRUE,
  FALSE,
  INT,
  FLOAT,
  CHAR,
  STRING,
  LPAREN,
  RPAREN,
  VARNAME,
  GETS,
  AS,
  FOR,
  IN,
  TO,
  STEP,
  NEXT,
  DO,
  LOOP,
  WHILE,
  WHEND,
  REPEAT,
  UNTIL,
  ON,
  GOSUB,
  END,
  ADD,
  SUB,
  MUL,
  DIV,
  ABS,
  ATAN,
  COS,
  EXP,
  LOG,
  RAND,
  SIN,
  SQR,
  TAN,
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
  { LIST,     .string = "list" },
  { PRINT,    .string = "print" },
  { INPUT,    .string = "input" },
  { DATA,     .string = "data" },
  { READ,     .string = "read" },
  { RESTORE,  .string = "restore" },
  { IF,       .string = "if" },
  { THEN,     .string = "then" },
  { AND,      .string = "and" },
  { OR,       .string = "or" },
  { TRUE,     .string = "true" },
  { FALSE,    .string = "false" },
  { INT,      .string = "int" },
  { FLOAT,    .string = "float" },
  { CHAR,     .string = "char" },
  { STRING,   .string = "string" },
  { AS,       .string = "as" },
  { FOR,      .string = "for" },
  { IN,       .string = "in" },
  { TO,       .string = "to" },
  { STEP,     .string = "step" },
  { NEXT,     .string = "next" },
  { DO,       .string = "do" },
  { LOOP,     .string = "loop" },
  { WHILE,    .string = "while" },
  { WHEND,    .string = "whend" },
  { REPEAT,   .string = "repeat" },
  { UNTIL,    .string = "until" },
  { ON,       .string = "on" },
  { GOSUB,    .string = "gosub" },
  { END,      .string = "end" },
  { ABS,      .string = "abs" },
  { ATAN,     .string = "atan" },
  { COS,      .string = "cos" },
  { EXP,      .string = "exp" },
  { LOG,      .string = "log" },
  { RAND,     .string = "rand" },
  { SIN,      .string = "sin" },
  { SQR,      .string = "sqr" },
  { TAN,      .string = "tan" },
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

