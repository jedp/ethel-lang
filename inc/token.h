#ifndef __TOKEN_H
#define __TOKEN_H

#include <inttypes.h>

typedef uint8_t tag_t;
enum tag_enum {
  TAG_EOF = 0,
  TAG_EOL,

  TAG_COMMENT,

  // Identifiers
  TAG_IDENT,
  TAG_ASSIGN,

  // Association
  TAG_LPAREN,
  TAG_RPAREN,
  TAG_COMMA,

  // Addition and multiplication
  TAG_PLUS,
  TAG_MINUS,
  TAG_TIMES,
  TAG_DIVIDE,
  TAG_MOD,
  TAG_AND,
  TAG_OR,

  // Types
  TAG_INT,
  TAG_FLOAT,
  TAG_CHAR,
  TAG_STRING,
  TAG_TRUE,
  TAG_FALSE,
  TAG_NIL,

  // Type declarations
  TAG_TYPE_INT,
  TAG_TYPE_FLOAT,
  TAG_TYPE_CHAR,
  TAG_TYPE_STRING,
  TAG_TYPE_BOOLEAN,
  TAG_AS,

  // Structure
  TAG_IF,
  TAG_THEN,
  TAG_ELSE,
  TAG_FOR,
  TAG_IN,
  TAG_TO,
  TAG_STEP,
  TAG_DO,

  // Comparison
  TAG_GT,
  TAG_LT,
  TAG_GE,
  TAG_LE,
  TAG_EQ,

  // IO
  TAG_PRINT,
  TAG_INPUT,

  // Blocks
  TAG_BEGIN,
  TAG_END,

  // Math
  TAG_ABS,
  TAG_SIN,
  TAG_COS,
  TAG_TAN,
  TAG_SQRT,
  TAG_EXP,
  TAG_LN,
  TAG_LOG,
};

static const char* tag_names[] = {
  "EOF",
  "EOL",

  "COMMENT",

  // Identifiers
  "IDENT",
  "ASSIGN",

  // Association
  "LPAREN",
  "RPAREN",
  "COMMA",

  // Addition and multiplication
  "PLUS",
  "MINUS",
  "TIMES",
  "DIVIDE",
  "MOD",
  "AND",
  "OR",

  // Types
  "INT",
  "FLOAT",
  "CHAR",
  "STRING",
  "TRUE",
  "FALSE",
  "NIL",

  // Type declarations
  "INT-TYPE",
  "FLOAT-TYPE",
  "CHAR-TYPE",
  "STRING-TYPE",
  "BOOLEAN-TYPE",
  "AS",

  // Structure
  "IF",
  "THEN",
  "ELSE",
  "FOR",
  "IN",
  "TO",
  "STEP",
  "DO",

  // Comparison
  "GT",
  "LT",
  "GE",
  "LE",
  "EQ",

  // IO
  "PRINT",
  "INPUT",

  // Blocks
  "BEGIN",
  "END",

  // Math
  "ABS",
  "SIN",
  "COS",
  "TAN",
  "SQR",
  "EXP",
  "LN",
  "LOG",
};

typedef struct {
  tag_t tag;
  union {
    int32_t intval;
    float floatval;
    char* string;
    char ch;
  } ;
} token_t;

static const token_t reserved[] = {
  { TAG_BEGIN,         .string = (char *) "begin" },
  { TAG_END,           .string = (char *) "end" },
  { TAG_IF,            .string = (char *) "if" },
  { TAG_THEN,          .string = (char *) "then" },
  { TAG_ELSE,          .string = (char *) "else" },
  { TAG_AND,           .string = (char *) "and" },
  { TAG_OR,            .string = (char *) "or" },
  { TAG_MOD,           .string = (char *) "mod" },
  { TAG_PRINT,         .string = (char *) "print" },
  { TAG_INPUT,         .string = (char *) "input" },
  { TAG_FOR,           .string = (char *) "for" },
  { TAG_IN,            .string = (char *) "in" },
  { TAG_TO,            .string = (char *) "to" },
  { TAG_STEP,          .string = (char *) "step" },
  { TAG_DO,            .string = (char *) "do" },
  { TAG_NIL,           .string = (char *) "nil" },
  { TAG_TYPE_INT,      .string = (char *) "int" },
  { TAG_TYPE_FLOAT,    .string = (char *) "float" },
  { TAG_TYPE_CHAR,     .string = (char *) "char" },
  { TAG_TYPE_STRING,   .string = (char *) "string" },
  { TAG_TYPE_BOOLEAN,  .string = (char *) "boolean" },
  { TAG_AS,            .string = (char *) "as" },
  { TAG_TRUE,          .string = (char *) "true" },
  { TAG_FALSE,         .string = (char *) "false" },
  { TAG_ABS,           .string = (char *) "abs" },
  { TAG_SIN,           .string = (char *) "sin" },
  { TAG_COS,           .string = (char *) "cos" },
  { TAG_TAN,           .string = (char *) "tan" },
  { TAG_SQRT,          .string = (char *) "sqrt" },
  { TAG_EXP,           .string = (char *) "exp" },
  { TAG_LN,            .string = (char *) "ln" },
  { TAG_LOG,           .string = (char *) "log" },
};

#endif

