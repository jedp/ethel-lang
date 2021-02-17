#ifndef __ERR_H
#define __ERR_H

typedef enum {
  NO_ERROR = 0,

  LEX_ERROR,

  ENV_MAX_DEPTH_EXCEEDED,
  ENV_NO_SCOPE,
  ENV_SYMBOL_REDEFINED,
  ENV_SYMBOL_UNDEFINED,

  TYPE_UNSUPPORTED,

  EVAL_UNHANDLED_OBJECT,
  EVAL_TYPE_ERROR,

  DIVISION_BY_ZERO,

  ERR_MAX,

} error_t;

static const char* err_names[ERR_MAX] = {
  "Ok",
  "Lexical error",
  "Max scope depth exceeded",
  "No current variable scope",
  "Immutable symbol cannot be redefined",
  "Symbol undefined",
  "Type unsupported",
  "Evaluation error",
  "Types incompatible for operation",
  "Division by zero",
};

#endif
