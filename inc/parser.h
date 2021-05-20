#ifndef __PARSER_H
#define __PARSER_H

#include "err.h"
#include "lexer.h"
#include "ast.h"

/*
 * Following the Principle of Least Astonishment.
 */
typedef enum {
  PRECED_NONE = 0, // Lowest precedence
  PRECED_CAST,
  PRECED_ASSIGN,
  PRECED_COND,
  PRECED_MAPS_TO,
  PRECED_OR,
  PRECED_AND,
  PRECED_EQ,
  PRECED_GLT,
  PRECED_BITWISE_OR,
  PRECED_BITWISE_XOR,
  PRECED_BITWISE_AND,
  PRECED_MEMBERSHIP,
  PRECED_RANGE,
  PRECED_ADD,
  PRECED_MUL,
  PRECED_BITWISE_SHIFT,
  PRECED_BITWISE_NOT,
  PRECED_NOT,
  PRECED_SUBSCRIPT,
  PRECED_FUNCTION_CALL,
  PRECED_MEMBER_ACCESS,
} preced_t;

typedef enum {
  ASSOC_LEFT = 0,
  ASSOC_RIGHT,
} assoc_t;

typedef struct {
  uint32_t pos;
  uint8_t depth;
  uint8_t err;
} parse_result_t;

void parse_program(char* input, ast_expr_t *ast, parse_result_t *parse_result);

#endif

