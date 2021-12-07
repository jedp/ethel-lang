#ifndef __DEF_H
#define __DEF_H

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#if (__WORDSIZE == 64)
#define BUILD64 1
#endif

#define True      1
#define False     0
#define Null      0

#define ETHEL_HEAP_SIZE_BYTES 16000000L
#define DICT_INIT_BUCKETS 16

// FNV (Fowlwer, Noll, Vo) FNV-1a 32-bit hash constants.
#define FNV32Prime 0x01000193
#define FNV32Basis 0x811C9DC5

enum flags {
  F_NONE           = 0x00,
  F_GC_FREE        = (1 << 1),
  F_ENV_ASSIGNABLE = (1 << 2),
  F_ENV_OVERWRITE  = (1 << 3),
  F_ENV_VAR        = (1 << 4),
};

enum every_type {
  TYPE_ERR_DO_NOT_USE = 0,
  AST_EMPTY,
  AST_ADD,
  AST_SUB,
  AST_MUL,
  AST_DIV,
  AST_MOD,
  AST_AND,
  AST_OR,
  AST_NOT,
  AST_BITWISE_SHL,
  AST_BITWISE_SHR,
  AST_BITWISE_AND,
  AST_BITWISE_OR,
  AST_BITWISE_XOR,
  AST_BITWISE_NOT,
  AST_GT,
  AST_GE,
  AST_LT,
  AST_LE,
  AST_EQ,
  AST_NE,
  AST_IS,
  AST_IN,
  AST_SUBSCRIPT,
  AST_MAPS_TO,
  AST_RANGE,
  AST_NIL,
  AST_LIST,
  AST_DICT,
  AST_APPLY,
  AST_INT,
  AST_FLOAT,
  AST_STRING,
  AST_BYTEARRAY_DECL,
  AST_BYTEARRAY,
  AST_BYTE,
  AST_BOOLEAN,
  AST_CAST,
  AST_ASSIGN,
  AST_DELETE,
  AST_NEGATE,
  AST_IDENT,
  AST_FIELD,
  AST_METHOD_CALL,
  AST_FUNCTION_DEF,
  AST_FUNCTION_CALL,
  AST_FUNCTION_RETURN,
  AST_TYPE_NAME,
  AST_BLOCK,
  AST_RESERVED_CALLABLE,
  AST_IF_THEN,
  AST_IF_THEN_ELSE,
  AST_DO_WHILE_LOOP,
  AST_WHILE_LOOP,
  AST_FOR_LOOP,
  AST_BREAK,
  AST_CONTINUE,
  AST_CALL_UNDEFINED,
  AST_CALL_TYPE_OF,
  AST_CALL_TO_HEX,
  AST_CALL_TO_BIN,
  AST_CALL_DUMP,
  AST_CALL_PRINT,
  AST_CALL_INPUT,
  AST_CALL_READ,
  AST_CALL_RAND,
  AST_CALL_ABS,
  AST_CALL_SIN,
  AST_CALL_COS,
  AST_CALL_TAN,
  AST_CALL_SQRT,
  AST_CALL_EXP,
  AST_CALL_LN,
  AST_CALL_LOG,
  TYPE_UNKNOWN,
  TYPE_NOTHING,
  TYPE_UNDEF,
  TYPE_NIL,
  TYPE_ERROR,
  TYPE_VARIABLE_ARGS,
  TYPE_FUNCTION,
  TYPE_FUNCTION_PTR_DATA,
  TYPE_RETURN_VAL,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_BYTE,
  TYPE_BYTEARRAY,
  TYPE_BYTEARRAY_DATA,
  TYPE_STRING,
  TYPE_BOOLEAN,
  TYPE_RANGE,
  TYPE_LIST,
  TYPE_LIST_DATA,
  TYPE_LIST_ELEM_DATA,
  TYPE_DICT,
  TYPE_DICT_DATA,
  TYPE_IDENT,
  TYPE_ITERATOR,
  TYPE_ITERATOR_DATA,
  TYPE_BREAK,
  TYPE_CONTINUE,
  ENV_SYM,
  TYPE_MAX,
};

static const char* type_names[TYPE_MAX] = {
  "Huge Mistake! Null pointer?",
  "<AST-EMPTY>",
  "AST-ADD",
  "AST-SUB",
  "AST-MUL",
  "AST-DIV",
  "AST-MOD",
  "AST-AND",
  "AST-OR",
  "AST-NOT",
  "AST-BITWISE-SHL",
  "AST-BITWISE-SHR",
  "AST-BITWISE-AND",
  "AST-BITWISE-OR",
  "AST-BITWISE-XOR",
  "AST-BITWISE-NOT",
  "AST-GT",
  "AST-GE",
  "AST-LT",
  "AST-LE",
  "AST-EQ",
  "AST-NE",
  "AST-IS-TYPE",
  "AST-MEMBER-OF",
  "AST-SUBSCRIPT",
  "AST-MAPS-TO",
  "AST-RANGE-FROM-TO",
  "AST-NIL",
  "AST-LIST",
  "AST-DICT",
  "AST-APPLY",
  "AST-INT",
  "AST-FLOAT",
  "AST-STRING",
  "AST-BYTE-ARRAY-DECLARATION",
  "AST-BYTE-ARRAY",
  "AST-BYTE",
  "AST-BOOLEAN",
  "AST-CAST",
  "AST-ASSIGN",
  "AST-DELETE",
  "AST-NEGATE",
  "AST-IDENT",
  "AST-FIELD",
  "AST-METHOD-CALL",
  "AST-FUNCTION-DEF",
  "AST-FUNCTION-CALL",
  "AST-FUNCTION-RETURN",
  "AST-TYPE-NAME",
  "AST-BLOCK",
  "AST-RESERVED-CALLABLE",
  "AST-IF-THEN",
  "AST-IF-THEN-ELSE",
  "AST-DO-WHILE-LOOP",
  "AST-WHILE-LOOP",
  "AST-FOR-LOOP",
  "AST-BREAK",
  "AST-CONTINUE",
  "AST-CALL-UNDEFINED",
  "AST-CALL-TYPE_OF",
  "AST-CALL-TO_HEX",
  "AST-CALL-TO_BIN",
  "AST-CALL-DUMP",
  "AST-CALL-PRINT",
  "AST-CALL-INPUT",
  "AST-CALL-READ",
  "AST-CALL-RAND",
  "AST-CALL-ABS",
  "AST-CALL-SIN",
  "AST-CALL-COS",
  "AST-CALL-TAN",
  "AST-CALL-SQRT",
  "AST-CALL-EXP",
  "AST-CALL-LN",
  "AST-CALL-LOG",
  "Unknown",
  "Nothing",
  "Undefined",
  "Nil",
  "Error",
  "Variable Args",
  "Function",
  "Function Data",
  "Return Val",
  "Int",
  "Float",
  "Byte",
  "Byte Array",
  "Byte Array Data",
  "Str",
  "Bool",
  "Range",
  "List",
  "List Data",
  "List Element Data",
  "Dict",
  "Dict Data",
  "Identifier",
  "Iterator",
  "Iterator Data",
  "Break",
  "Continue",
  "Environment Symbol",
};

typedef uint8_t byte;
typedef uint32_t boolean;
typedef uint8_t flags_t;
typedef uint8_t type_t;

typedef struct {
#ifdef BUILD64
  uint64_t type : 52;
#else
  int type : 20;
#endif
  int flags : 8;
  int children : 4;
} gc_header_t;

typedef struct {
  gc_header_t hdr;
  size_t size;
  byte data[];
} bytearray_t;

#endif

