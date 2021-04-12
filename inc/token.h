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
  TAG_TYPE_NAME,
  TAG_ASSIGN,
  TAG_INVARIABLE,
  TAG_VARIABLE,
  TAG_DEL,

  // Association
  TAG_LPAREN,
  TAG_RPAREN,
  TAG_COMMA,

  // Arithmetic
  TAG_PLUS,
  TAG_MINUS,
  TAG_TIMES,
  TAG_DIVIDE,
  TAG_MOD,
  TAG_AND,
  TAG_OR,
  TAG_BITWISE_SHL,
  TAG_BITWISE_SHR,
  TAG_BITWISE_AND,
  TAG_BITWISE_OR,
  TAG_BITWISE_XOR,
  TAG_BITWISE_NOT,

  // Types
  TAG_BIN,
  TAG_HEX,
  TAG_INT,
  TAG_FLOAT,
  TAG_CHAR,
  TAG_STRING,
  TAG_TRUE,
  TAG_FALSE,
  TAG_NIL,
  TAG_LIST,
  TAG_OF,

  // Array constructor
  TAG_ARR_DECL,

  // Type declarations
  TAG_TYPE_BIN,
  TAG_TYPE_HEX,
  TAG_TYPE_INT,
  TAG_TYPE_FLOAT,
  TAG_TYPE_CHAR,
  TAG_TYPE_STRING,
  TAG_TYPE_BOOLEAN,
  TAG_AS,

  // Sequence
  TAG_RANGE,

  // Element access
  TAG_LBRACKET,
  TAG_RBRACKET,

  // Structure
  TAG_IF,
  TAG_THEN,
  TAG_ELSE,
  TAG_WHILE,
  TAG_FOR,
  TAG_IN,
  TAG_STEP,

  // Comparison
  TAG_GT,
  TAG_LT,
  TAG_GE,
  TAG_LE,
  TAG_EQ,
  TAG_NE,

  // IO
  TAG_DUMP,
  TAG_PRINT,
  TAG_INPUT,

  // Blocks
  TAG_BEGIN,
  TAG_END,

  // Fields and methods
  TAG_MEMBER_ACCESS,
  TAG_FIELD_NAME,
  TAG_METHOD_NAME,

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
  "TYPE-NAME",
  "ASSIGN",
  "INVARIABLE",
  "VARIABLE",
  "DELETE",

  // Association
  "LPAREN",
  "RPAREN",
  "COMMA",

  // Arithmetic
  "PLUS",
  "MINUS",
  "TIMES",
  "DIVIDE",
  "MOD",
  "AND",
  "OR",
  "BITWISE_SHL",
  "BITWISE_SHR",
  "BITWISE_AND",
  "BITWISE_OR",
  "BITWISE_XOR",
  "BITWISE_NOT",

  // Types
  "HEX",
  "INT",
  "FLOAT",
  "CHAR",
  "STRING",
  "TRUE",
  "FALSE",
  "NIL",
  "LIST",
  "OF",

  // Array constructor
  "ARRAY-DECL",

  // Type declarations
  "INT-TYPE",
  "FLOAT-TYPE",
  "CHAR-TYPE",
  "STRING-TYPE",
  "BOOLEAN-TYPE",
  "AS",

  // Sequence
  "RANGE-TO",

  // Element access
  "OPEN_BRACKET",
  "CLOSE_BRACKET",

  // Structure
  "IF",
  "THEN",
  "ELSE",
  "WHILE",
  "FOR",
  "IN",
  "STEP",

  // Comparison
  "GT",
  "LT",
  "GE",
  "LE",
  "EQ",
  "NE",

  // IO
  "DUMP",
  "PRINT",
  "INPUT",

  // Blocks
  "BEGIN",
  "END",

  // Field and method access
  "MEMBER-ACCESS",
  "FIELD-NAME",
  "METHOD-NAME",

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
  { TAG_IF,            .string = (char *) "if" },
  { TAG_THEN,          .string = (char *) "then" },
  { TAG_ELSE,          .string = (char *) "else" },
  { TAG_AND,           .string = (char *) "and" },
  { TAG_OR,            .string = (char *) "or" },
  { TAG_MOD,           .string = (char *) "mod" },
  { TAG_INVARIABLE,    .string = (char *) "val" },
  { TAG_VARIABLE,      .string = (char *) "var" },
  { TAG_DEL,           .string = (char *) "del" },
  { TAG_DUMP,          .string = (char *) "dump" },
  { TAG_PRINT,         .string = (char *) "print" },
  { TAG_INPUT,         .string = (char *) "input" },
  { TAG_WHILE,         .string = (char *) "while" },
  { TAG_FOR,           .string = (char *) "for" },
  { TAG_IN,            .string = (char *) "in" },
  { TAG_STEP,          .string = (char *) "step" },
  { TAG_NIL,           .string = (char *) "nil" },
  { TAG_LIST,          .string = (char *) "list" },
  { TAG_OF,            .string = (char *) "of" },
  { TAG_ARR_DECL,      .string = (char *) "arr" },
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

