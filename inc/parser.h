#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.h"
#include "ast.h"

typedef enum {
  PRECED_NONE = 0,
  PRECED_ASSIGN,
  PRECED_COND,
  PRECED_OR,
  PRECED_AND,
  PRECED_EQ,
  PRECED_GLT,
  PRECED_SHIFT,
  PRECED_ADD,
  PRECED_MUL,
  PRECED_CAST,
  PRECED_UNARY,
} preced_t;

typedef enum {
  ASSOC_LEFT = 0,
  ASSOC_RIGHT,
} assoc_t;

ast_expr_t *parse_program(char* input);

#endif

