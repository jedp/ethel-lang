#ifndef _AST_H
#define _AST_H

#include <inttypes.h>

typedef enum {
    AST_EMPTY = 0,
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_INT,
    AST_FLOAT,
    AST_IDENT,
} ast_type_t;

static const char *ast_node_names[] = {
    "<EMPTY>",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "INT",
    "FLOAT",
    "IDENT",
};

typedef enum {
    AST_NO_ERROR,
    AST_SYNTAX_ERROR,
    AST_UNSUPPORTED_OPERATION,
} ast_err_t;

typedef struct Expr {
    uint8_t type;
    void *e1;
    void *e2;
    union {
        int intval;
        float floatval;
    };
} ast_expr_t;

void pretty_print(ast_expr_t *expr);
ast_expr_t *ast_expr(ast_type_t type, ast_expr_t *e1, ast_expr_t *e2);
ast_expr_t *ast_float(float value);
ast_expr_t *ast_int(int value);
ast_expr_t *ast_empty();

#endif

