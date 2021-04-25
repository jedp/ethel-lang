#ifndef __ERR_H
#define __ERR_H

#include <inttypes.h>

typedef uint8_t error_t;
enum error_enum {
  ERR_NO_ERROR = 0,
  ERR_LEX_ERROR,
  ERR_LEX_UNEXPECTED_TOKEN,
  ERR_LEX_INCOMPLETE_INPUT,
  ERR_AST_TYPE_UNHANDLED,
  ERR_ENV_MAX_DEPTH_EXCEEDED,
  ERR_ENV_NO_SCOPE,
  ERR_ENV_SYMBOL_REDEFINED,
  ERR_ENV_SYMBOL_UNDEFINED,
  ERR_TYPE_UNSUPPORTED,
  ERR_TYPE_INT_REQUIRED,
  ERR_TYPE_SEQUENCE_REQUIRED,
  ERR_TYPE_ITERABLE_REQUIRED,
  ERR_TYPE_POSITIVE_INT_REQUIRED,
  ERR_EVAL_UNHANDLED_OBJECT,
  ERR_EVAL_TYPE_ERROR,
  ERR_EVAL_BAD_INPUT,
  ERR_DIVISION_BY_ZERO,
  ERR_OVERFLOW_ERROR,
  ERR_UNDERFLOW_ERROR,
  ERR_VALUE_TOO_LARGE_FOR_BYTE,
  ERR_INPUT_STREAM_ERROR,
  ERR_INPUT_TOO_LONG,
  ERR_INDEX_OUT_OF_RANGE,
  ERR_RANGE_ERROR,
  ERR_NO_SUCH_FIELD,
  ERR_NO_SUCH_METHOD,
  ERR_WRONG_ARG_COUNT,
  ERR_SYNTAX_ERROR,
  ERR_MAX
};

static const char* err_names[ERR_MAX] = {
  "Ok",
  "Lexical error",
  "Unexpected token",
  "Incomplete input",
  "Parser error: Unhandled type!",
  "Max scope depth exceeded",
  "No current variable scope!",
  "Cannot redefine immutable object",
  "Symbol undefined",
  "Type unsupported",
  "Integer required",
  "Not a sequence type",
  "Type error: Iterable required",
  "Positive integer required",
  "Evaluation error: Unhandled object!",
  "Type error",
  "Bad input",
  "Division by zero",
  "Overflow error",
  "Underflow error",
  "Value too large for byte",
  "Input stream error",
  "Input too long",
  "Index out of range",
  "Range error",
  "No such field",
  "No such method",
  "Incorrect number of arguments",
  "Syntax error",
};

#endif
