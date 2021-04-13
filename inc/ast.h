#ifndef _AST_H
#define _AST_H

#include <inttypes.h>
#include "def.h"

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
  AST_RANGE,
  AST_NIL,
  AST_LIST,
  AST_APPLY,
  AST_INT,
  AST_FLOAT,
  AST_STRING,
  AST_BYTEARRAY_DECL,
  AST_BYTEARRAY,
  AST_CHAR,
  AST_BOOLEAN,
  AST_CAST,
  AST_ASSIGN,
  AST_REASSIGN,
  AST_DELETE,
  AST_NEGATE,
  AST_IDENT,
  AST_SEQ_ELEM,
  AST_SEQ_ELEM_ASSIGN,
  AST_FIELD,
  AST_METHOD_CALL,
  AST_FUNCTION_DEF,
  AST_FUNCTION_CALL,
  AST_TYPE_NAME,
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
  "BITWISE-SHL",
  "BITWISE-SHR",
  "BITWISE-AND",
  "BITWISE-OR",
  "BITWISE-XOR",
  "BITWISE-NOT",
  "GT",
  "GE",
  "LT",
  "LE",
  "EQ",
  "NE",
  "RANGE-FROM-TO",
  "NIL",
  "LIST",
  "APPLY",
  "INT",
  "FLOAT",
  "STRING",
  "BYTE-ARRAY-DECLARATION",
  "BYTE-ARRAY",
  "CHAR",
  "BOOLEAN",
  "CAST",
  "ASSIGN",
  "REASSIGN",
  "DELETE",
  "NEGATE",
  "IDENT",
  "SEQUENCE-ELEM",
  "SEQUENCE-ELEM-ASSIGN",
  "FIELD",
  "METHOD-CALL",
  "FUNCTION-DEF",
  "FUNCTION-CALL",
  "TYPE-NAME",
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
  AST_CALL_TO_HEX,
  AST_CALL_TO_BIN,
  AST_CALL_DUMP,
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

typedef struct AstExpr ast_expr_t;

typedef struct AstAssign {
  ast_expr_t *ident;
  ast_expr_t *value;
} ast_assign_t;

typedef struct AstUnaryArg {
  ast_expr_t *a;
} ast_unary_arg_t;

typedef struct AstBinOpArgs {
  ast_expr_t *a;
  ast_expr_t *b;
} ast_binop_args_t;

typedef struct AstRangeArgs {
  ast_expr_t *from;
  ast_expr_t *to;
} ast_range_args_t;

typedef struct AstCastArgs {
  ast_expr_t *a;
  ast_expr_t *b;
} ast_cast_args_t;

typedef struct AstIfThenArgs {
  ast_expr_t *cond;
  ast_expr_t *pred;
} ast_if_then_args_t;

typedef struct AstIfThenElseArgs {
  ast_expr_t *cond;
  ast_expr_t *pred;
  ast_expr_t *else_pred;
} ast_if_then_else_args_t;

typedef struct AstExprListNode {
  ast_expr_t *root;
  struct AstExprListNode *next;
} ast_expr_list_t;

typedef struct AstList {
  char* type_name;
  ast_expr_list_t *es;
} ast_list_t;

typedef struct AstReservedCallable {
  ast_reserved_callable_type_t type;
  ast_expr_list_t *es;
} ast_reserved_callable_t;

typedef struct AstWhileLoop {
  ast_expr_t *cond;
  ast_expr_t *pred;
} ast_while_loop_t;

typedef struct AstForLoop {
  ast_expr_t *index;
  ast_expr_t *range;
  ast_expr_t *pred;
} ast_for_loop_t;

typedef struct AstVarNameMap {
  char* name;
  ast_expr_t *value;
} ast_var_name_map_t;

typedef struct AstFnArgDecl {
  char *name;
  struct AstFnArgDecl *next;
} ast_fn_arg_decl_t;

typedef struct AstFunc {
  ast_fn_arg_decl_t *argnames;
  ast_expr_list_t *block_exprs;
} ast_func_def_t;

typedef struct AstFuncCall {
  bytearray_t *name;
  ast_expr_list_t *args;
} ast_func_call_t;

typedef struct AstArrayDecl {
  ast_expr_t *size;
} ast_array_decl_t;

typedef struct AstSeqElem {
  ast_expr_t *ident;
  ast_expr_t *index;
} ast_seq_elem_t;

typedef struct AstAssignElem {
  ast_expr_t *seq;
  ast_expr_t *offset;
  ast_expr_t *value;
} ast_assign_elem_t;

typedef struct AstField {
  char* name;
} ast_field_t;

typedef struct AstMethod {
  char* name;
  ast_type_t result_type;
  ast_expr_list_t *args;
} ast_method_t;

typedef struct AstApply {
  ast_expr_t *receiver;
  char* member_name;
  ast_expr_list_t *args;
} ast_apply_t;

typedef struct __attribute__((__packed__)) AstExpr {
  uint16_t type;
  uint16_t flags;
  union {
    ast_assign_t *assignment;
    ast_expr_list_t *block_exprs;
    ast_list_t *list;
    ast_unary_arg_t *unary_arg;
    ast_binop_args_t *binop_args;
    ast_range_args_t *range;
    ast_cast_args_t *cast_args;
    ast_reserved_callable_t *reserved_callable;
    ast_if_then_args_t *if_then_args;
    ast_if_then_else_args_t *if_then_else_args;
    ast_while_loop_t *while_loop;
    ast_for_loop_t *for_loop;
    ast_array_decl_t *array_decl;
    ast_seq_elem_t *seq_elem;
    ast_assign_elem_t *assign_elem;
    ast_method_t *method_call;
    ast_apply_t *application;
    ast_func_def_t *func_def;
    ast_func_call_t *func_call;
    int intval;
    int boolval;
    float floatval;
    char charval;
    bytearray_t *bytearray;
  };
} ast_expr_t;

void pretty_print(ast_expr_t *expr);
ast_expr_t *ast_unary(ast_type_t type, ast_expr_t *a);
ast_expr_t *ast_binop(ast_type_t type, ast_expr_t *a, ast_expr_t *b);
ast_expr_t *ast_cast(ast_expr_t *e1, ast_expr_t *e2);
ast_expr_t *ast_nil();
ast_expr_t *ast_list(char* type_name, ast_expr_list_t *nullable_init_es);
ast_expr_t *ast_float(float value);
ast_expr_t *ast_int(int value);
ast_expr_t *ast_char(char c);
ast_expr_t *ast_string(char* s);
ast_expr_t *ast_array_decl(ast_expr_t *size);
ast_expr_t *ast_assign_elem(ast_expr_t *seq, ast_expr_t *offset, ast_expr_t *value);
ast_expr_t *ast_boolean(boolean t);
ast_expr_t *ast_type(ast_type_t type);
ast_expr_t *ast_ident(char* name);
ast_expr_t *ast_seq_elem(ast_expr_t *ident, ast_expr_t *index);
ast_expr_t *ast_field(char* name);
ast_expr_t *ast_func_def(ast_fn_arg_decl_t *args, ast_expr_list_t *es);
ast_expr_t *ast_func_call(bytearray_t *name, ast_expr_list_t *args);
ast_expr_t *ast_method_call(char* name, ast_expr_list_t *args);
ast_expr_t *ast_member_access(ast_expr_t *receiver, char* member_name, ast_expr_list_t *args);
ast_expr_t *ast_type_name(char* name);
ast_expr_t *ast_range(ast_expr_t *from, ast_expr_t *to);
ast_expr_t *ast_access(ast_expr_t *object, ast_expr_t *member);
ast_expr_t *ast_block(ast_expr_list_t *es);
ast_expr_t *ast_reserved_callable(ast_reserved_callable_type_t type, ast_expr_list_t *es);
ast_expr_t *ast_assign_var(ast_expr_t *ident, ast_expr_t *value);
ast_expr_t *ast_assign_val(ast_expr_t *ident, ast_expr_t *value);
ast_expr_t *ast_reassign(ast_expr_t *ident, ast_expr_t *value);
ast_expr_t *ast_delete(ast_expr_t *ident);
ast_expr_t *ast_if_then(ast_expr_t *if_clause, ast_expr_t *then_clause);
ast_expr_t *ast_if_then_else(ast_expr_t *if_clause, ast_expr_t *then_clause, ast_expr_t *else_clause);
ast_expr_t *ast_while_loop(ast_expr_t *cond, ast_expr_t *pred);
ast_expr_t *ast_for_loop(ast_expr_t *index, ast_expr_t *range, ast_expr_t *pred);
ast_expr_t *ast_empty();

#endif

