#ifndef _AST_H
#define _AST_H

#include <inttypes.h>
#include <stdbool.h>

typedef uint8_t ast_type_t;
enum ast_type_enum {
  AST_EMPTY = 0,
  AST_ADD,
  AST_SUB,
  AST_MUL,
  AST_DIV,
  AST_MOD,
  AST_AND,
  AST_OR,
  AST_GT,
  AST_GE,
  AST_LT,
  AST_LE,
  AST_EQ,
  AST_NE,
  AST_RANGE,
  AST_NIL,
  AST_INT,
  AST_FLOAT,
  AST_STRING,
  AST_CHAR,
  AST_BOOLEAN,
  AST_CAST,
  AST_ASSIGN,
  AST_DELETE,
  AST_NEGATE,
  AST_IDENT,
  AST_BLOCK,
  AST_RESERVED_CALLABLE,
  AST_IF_THEN,
  AST_IF_THEN_ELSE,
  AST_WHILE_LOOP,
  AST_FOR_LOOP,
};

static const char *ast_node_names[] = {
  "<EMPTY>",
  "ADD",
  "SUB",
  "MUL",
  "DIV",
  "MOD",
  "AND",
  "OR",
  "GT",
  "GE",
  "LT",
  "LE",
  "EQ",
  "NE",
  "RANGE-FROM-TO",
  "NIL",
  "INT",
  "FLOAT",
  "STRING",
  "CHAR",
  "BOOLEAN",
  "CAST",
  "ASSIGN",
  "DELETE",
  "NEGATE",
  "IDENT",
  "BLOCK",
  "RESERVED-CALLABLE",
  "IF-THEN",
  "IF-THEN-ELSE",
  "WHILE-LOOP",
  "FOR-LOOP",
};

typedef uint8_t ast_reserved_callable_type_t;
enum ast_call_type_enum {
  AST_CALL_UNDEFINED = 0,
  AST_CALL_PRINT,
  AST_CALL_INPUT,
  AST_CALL_ABS,
  AST_CALL_SIN,
  AST_CALL_COS,
  AST_CALL_TAN,
  AST_CALL_SQRT,
  AST_CALL_EXP,
  AST_CALL_LN,
  AST_CALL_LOG,
};

typedef struct Expr ast_expr_t;

typedef struct Assign {
  ast_expr_t *ident;
  ast_expr_t *value;
} ast_assign_t;

typedef struct BinOpArgs {
  ast_expr_t *a;
  ast_expr_t *b;
} ast_binop_args_t;

typedef struct RangeArgs {
  ast_expr_t *from;
  ast_expr_t *to;
} ast_range_args_t;

typedef struct CastArgs {
  ast_expr_t *a;
  ast_expr_t *b;
} ast_cast_args_t;

typedef struct IfThenArgs {
  ast_expr_t *cond;
  ast_expr_t *pred;
} ast_if_then_args_t;

typedef struct IfThenElseArgs {
  ast_expr_t *cond;
  ast_expr_t *pred;
  ast_expr_t *else_pred;
} ast_if_then_else_args_t;

typedef struct ExprListNode {
  ast_expr_t *root;
  struct ExprListNode *next;
} ast_expr_list_t;

typedef struct WhileLoop {
  ast_expr_t *cond;
  ast_expr_t *pred;
} ast_while_loop_t;

typedef struct ForLoop {
  ast_expr_t *index;
  ast_expr_t *range;
  ast_expr_t *pred;
} ast_for_loop_t;

typedef struct Expr {
  uint8_t type;
  uint8_t flags;
  union {
    ast_assign_t *assignment;
    ast_expr_list_t *block_exprs;
    ast_binop_args_t *binop_args;
    ast_range_args_t *range;
    ast_cast_args_t *cast_args;
    ast_if_then_args_t *if_then_args;
    ast_if_then_else_args_t *if_then_else_args;
    ast_while_loop_t *while_loop;
    ast_for_loop_t *for_loop;
    int intval;
    float floatval;
    char* stringval;
    char charval;
  };
} ast_expr_t;

void pretty_print(ast_expr_t *expr);
ast_expr_t *ast_binop(ast_type_t type, ast_expr_t *a, ast_expr_t *b);
ast_expr_t *ast_cast(ast_expr_t *e1, ast_expr_t *e2);
ast_expr_t *ast_nil();
ast_expr_t *ast_float(float value);
ast_expr_t *ast_int(int value);
ast_expr_t *ast_char(char c);
ast_expr_t *ast_string(char* s);
ast_expr_t *ast_boolean(bool t);
ast_expr_t *ast_type(ast_type_t type);
ast_expr_t *ast_ident(char* name);
ast_expr_t *ast_range(ast_expr_t *from, ast_expr_t *to);
ast_expr_t *ast_block(ast_expr_list_t *es);
ast_expr_t *ast_reserved_callable(ast_reserved_callable_type_t type, ast_expr_list_t *es);
ast_expr_t *ast_assign(ast_expr_t *ident, ast_expr_t *value, uint8_t flags);
ast_expr_t *ast_delete(ast_expr_t *ident);
ast_expr_t *ast_if_then(ast_expr_t *if_clause, ast_expr_t *then_clause);
ast_expr_t *ast_if_then_else(ast_expr_t *if_clause, ast_expr_t *then_clause, ast_expr_t *else_clause);
ast_expr_t *ast_while_loop(ast_expr_t *cond, ast_expr_t *pred);
ast_expr_t *ast_for_loop(ast_expr_t *index, ast_expr_t *range, ast_expr_t *pred);
ast_expr_t *ast_empty();

#endif

