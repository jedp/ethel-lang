#ifndef _AST_H
#define _AST_H

#include <inttypes.h>

typedef enum {
  AST_EMPTY = 0,
  AST_ADD,
  AST_SUB,
  AST_MUL,
  AST_DIV,
  AST_NIL,
  AST_INT,
  AST_FLOAT,
  AST_STRING,
  AST_CHAR,
  AST_ASSIGN,
  AST_NEGATE,
  AST_IDENT,
  AST_RESERVED_CALLABLE,
  AST_IF_THEN,
  AST_IF_THEN_ELSE,
} ast_type_t;

static const char *ast_node_names[] = {
  "<EMPTY>",
  "ADD",
  "SUB",
  "MUL",
  "DIV",
  "NIL",
  "INT",
  "FLOAT",
  "STRING",
  "CHAR",
  "ASSIGN",
  "NEGATE",
  "IDENT",
  "RESERVED-CALLABLE"
  "IF-THEN",
  "IF-THEN-ELSE",
};

typedef enum {
  AST_CALL_UNDEFINED = 0,
  AST_CALL_ABS,
  AST_CALL_SIN,
  AST_CALL_COS,
  AST_CALL_TAN,
  AST_CALL_SQRT,
  AST_CALL_EXP,
  AST_CALL_LN,
  AST_CALL_LOG,
} ast_reserved_callable_type_t;

typedef struct Expr {
  uint8_t type;
  void *e1;
  void *e2;
  void *e3;
  union {
    int intval;
    float floatval;
    char* stringval;
    char charval;
  };
} ast_expr_t;

typedef struct ExprListNode {
  ast_expr_t *e;
  struct ExprListNode *next;
} ast_expr_list_t;

void pretty_print(ast_expr_t *expr);
ast_expr_t *ast_expr(ast_type_t type, ast_expr_t *e1, ast_expr_t *e2);
ast_expr_t *ast_nil();
ast_expr_t *ast_float(float value);
ast_expr_t *ast_int(int value);
ast_expr_t *ast_char(char c);
ast_expr_t *ast_string(char* s);
ast_expr_t *ast_ident(char* name);
ast_expr_t *ast_reserved_callable(ast_reserved_callable_type_t type, ast_expr_list_t *es);
ast_expr_t *ast_assign(ast_expr_t *ident, ast_expr_t *value);
ast_expr_t *ast_if_then(ast_expr_t *if_clause, ast_expr_t *then_clause);
ast_expr_t *ast_if_then_else(ast_expr_t *if_clause, ast_expr_t *then_clause, ast_expr_t *else_clause);
ast_expr_t *ast_empty();

#endif

