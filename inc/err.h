#ifndef __ERR_H
#define __ERR_H

#include <inttypes.h>

typedef uint8_t error_t;
enum error_enum {
  NO_ERROR = 0,

  LEX_ERROR,
  LEX_UNEXPECTED_TOKEN,
  LEX_INCOMPLETE_INPUT,

  AST_TYPE_UNHANDLED,

  ENV_MAX_DEPTH_EXCEEDED,
  ENV_NO_SCOPE,
  ENV_SYMBOL_REDEFINED,
  ENV_SYMBOL_UNDEFINED,

  TYPE_UNSUPPORTED,
  TYPE_INT_REQUIRED,
  TYPE_SEQUENCE_REQUIRED,
  TYPE_POSITIVE_INT_REQUIRED,

  EVAL_UNHANDLED_OBJECT,
  EVAL_TYPE_ERROR,
  EVAL_BAD_INPUT,

  DIVISION_BY_ZERO,
  OVERFLOW_ERROR,
  UNDERFLOW_ERROR,
  VALUE_TOO_LARGE_FOR_CHAR,
  INPUT_STREAM_ERROR,
  INPUT_TOO_LONG,
  INDEX_OUT_OF_RANGE,

  NO_SUCH_FIELD,
  NO_SUCH_METHOD,

  SYNTAX_ERROR,

  ERR_MAX,
};

static const char* err_names[ERR_MAX] = {
  "Ok",
  "Lexical error",
  "Unexpected token",
  "Incomplete input",
  "Parser error",
  "Max scope depth exceeded",
  "No current variable scope",
  "Immutable symbol cannot be redefined",
  "Symbol undefined",
  "Type unsupported",
  "Integer argument required",
  "Sequence type required for operation",
  "Positive integer argument required",
  "Evaluation error",
  "Types incompatible for operation",
  "Bad input",
  "Division by zero",
  "Overflow error",
  "Underflow error",
  "Char value too large",
  "Input stream error",
  "Input too long",
  "Index out of range",
  "No such field",
  "No such method",
  "Syntax error",
};

#endif
