#ifndef __ERR_H
#define __ERR_H

typedef enum {
  NO_ERROR = 0,

  LEX_ERROR,

  AST_TYPE_UNHANDLED,

  ENV_MAX_DEPTH_EXCEEDED,
  ENV_NO_SCOPE,
  ENV_SYMBOL_REDEFINED,
  ENV_SYMBOL_UNDEFINED,

  TYPE_UNSUPPORTED,
  TYPE_INT_REQUIRED,
  TYPE_POSITIVE_INT_REQUIRED,

  EVAL_UNHANDLED_OBJECT,
  EVAL_TYPE_ERROR,
  EVAL_BAD_INPUT,

  DIVISION_BY_ZERO,
  OVERFLOW_ERROR,
  UNDERFLOW_ERROR,
  VALUE_TOO_LARGE_FOR_CHAR,

  ERR_MAX,

} error_t;

static const char* err_names[ERR_MAX] = {
  "Ok",
  "Lexical error",
  "Parser broken: Type not handled",
  "Max scope depth exceeded",
  "No current variable scope",
  "Immutable symbol cannot be redefined",
  "Symbol undefined",
  "Type unsupported",
  "Integer argument required",
  "Positive integer argument required",
  "Evaluation error",
  "Types incompatible for operation",
  "Bad input",
  "Division by zero",
  "Overflow error",
  "Underflow error",
  "Char value too large",
};

#endif
