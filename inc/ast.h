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
  "NOT",
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
  "IS-TYPE",
  "MEMBER-OF",
  "SUBSCRIPT",
  "MAPS-TO",
  "RANGE-FROM-TO",
  "NIL",
  "LIST",
  "DICT",
  "APPLY",
  "INT",
  "FLOAT",
  "STRING",
  "BYTE-ARRAY-DECLARATION",
  "BYTE-ARRAY",
  "BYTE",
  "BOOLEAN",
  "CAST",
  "ASSIGN",
  "DELETE",
  "NEGATE",
  "IDENT",
  "FIELD",
  "METHOD-CALL",
  "FUNCTION-DEF",
  "FUNCTION-CALL",
  "FUNCTION-RETURN",
  "TYPE-NAME",
  "BLOCK",
  "RESERVED-CALLABLE",
  "IF-THEN",
  "IF-THEN-ELSE",
  "DO-WHILE-LOOP",
  "WHILE-LOOP",
  "FOR-LOOP",
  "BREAK",
  "CONTINUE",
};

typedef uint8_t ast_reserved_callable_type_t;
enum ast_call_type_enum {
  AST_CALL_UNDEFINED = 0,
  AST_CALL_TYPE_OF,
  AST_CALL_TO_HEX,
  AST_CALL_TO_BIN,
  AST_CALL_DUMP,
  AST_CALL_PRINT,
  AST_CALL_INPUT,
  AST_CALL_RAND,
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

typedef struct AstOpArgs {
  ast_expr_t *a;
  ast_expr_t *b;
} ast_op_args_t;

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
  ast_expr_list_t *es;
} ast_list_t;

typedef struct AstExprKvListNode {
  ast_expr_t *k;
  ast_expr_t *v;
  struct AstExprKvListNode *next;
} ast_expr_kv_list_t;

typedef struct AstDict {
  ast_expr_kv_list_t *kv;
} ast_dict_t;

typedef struct AstReservedCallable {
  ast_reserved_callable_type_t type;
  ast_expr_list_t *es;
} ast_reserved_callable_t;

typedef struct AstDoWhileLoop {
  ast_expr_t *pred;
  ast_expr_t *cond;
} ast_do_while_loop_t;

typedef struct AstWhileLoop {
  ast_expr_t *cond;
  ast_expr_t *pred;
} ast_while_loop_t;

typedef struct AstForLoop {
  ast_expr_t *elem;
  ast_expr_t *iterable;
  ast_expr_t *pred;
} ast_for_loop_t;

typedef struct AstVarNameMap {
  bytearray_t *name;
  ast_expr_t *value;
} ast_var_name_map_t;

typedef struct AstFnArgDecl {
  bytearray_t *name;
  struct AstFnArgDecl *next;
} ast_fn_arg_decl_t;

typedef struct AstFunc {
  ast_fn_arg_decl_t *argnames;
  ast_expr_list_t *block_exprs;
} ast_func_def_t;

typedef struct AstFuncCall {
  ast_expr_t *expr;
  ast_expr_list_t *args;
} ast_func_call_t;

typedef struct AstArrayDecl {
  ast_expr_t *size;
} ast_array_decl_t;

typedef struct AstAssignElem {
  ast_expr_t *seq;
  ast_expr_t *offset;
  ast_expr_t *value;
} ast_assign_elem_t;

typedef struct AstField {
  bytearray_t *name;
} ast_field_t;

typedef struct AstMethod {
  bytearray_t *name;
  ast_type_t result_type;
  ast_expr_list_t *args;
} ast_method_t;

typedef struct AstApply {
  ast_expr_t *receiver;
  bytearray_t *member_name;
  ast_expr_list_t *args;
} ast_apply_t;

typedef struct __attribute__((__packed__)) AstExpr {
  uint16_t type;
  uint16_t flags;
  union {
    ast_assign_t *assignment;
    ast_expr_list_t *block_exprs;
    ast_list_t *list;
    ast_dict_t *dict;
    ast_unary_arg_t *unary_arg;
    ast_op_args_t *op_args;
    ast_range_args_t *range;
    ast_cast_args_t *cast_args;
    ast_reserved_callable_t *reserved_callable;
    ast_if_then_args_t *if_then_args;
    ast_if_then_else_args_t *if_then_else_args;
    ast_do_while_loop_t *do_while_loop;
    ast_while_loop_t *while_loop;
    ast_for_loop_t *for_loop;
    ast_array_decl_t *array_decl;
    ast_assign_elem_t *assign_elem;
    ast_method_t *method_call;
    ast_apply_t *application;
    ast_func_def_t *func_def;
    ast_func_call_t *func_call;
    ast_expr_list_t *func_return_values;
    int intval;
    int boolval;
    float floatval;
    byte byteval;
    bytearray_t *bytearray;
  };
} ast_expr_t;

void pretty_print(ast_expr_t *expr);
ast_expr_t *ast_unary(ast_type_t type, ast_expr_t *a);
ast_expr_t *ast_op(ast_type_t type, ast_expr_t *a, ast_expr_t *b);
ast_expr_t *ast_cast(ast_expr_t *e1, ast_expr_t *e2);
ast_expr_t *ast_nil(void);
ast_expr_t *ast_list(ast_expr_list_t *nullable_init_es);
ast_expr_t *ast_dict(ast_expr_kv_list_t *nullable_kvs);
ast_expr_t *ast_float(float value);
ast_expr_t *ast_int(int value);
ast_expr_t *ast_byte(byte b);
ast_expr_t *ast_string(bytearray_t *s);
ast_expr_t *ast_array_decl(ast_expr_t *size);
ast_expr_t *ast_assign_elem(ast_expr_t *seq, ast_expr_t *offset, ast_expr_t *value);
ast_expr_t *ast_boolean(boolean t);
ast_expr_t *ast_type(ast_type_t type);
ast_expr_t *ast_ident(bytearray_t *name);
ast_expr_t *ast_ident_decl(bytearray_t *name, uint16_t flags);
ast_expr_t *ast_field(bytearray_t *s);
ast_expr_t *ast_func_def(ast_fn_arg_decl_t *args, ast_expr_list_t *es);
ast_expr_t *ast_func_call(ast_expr_t *expr, ast_expr_list_t *args);
ast_expr_t *ast_func_return(ast_expr_list_t *func_return_values);
ast_expr_t *ast_method_call(bytearray_t *name, ast_expr_list_t *args);
ast_expr_t *ast_member_access(ast_expr_t *receiver,
                              bytearray_t *meber_name,
                              ast_expr_list_t *args);
ast_expr_t *ast_type_name(bytearray_t *name);
ast_expr_t *ast_range(ast_expr_t *from, ast_expr_t *to);
ast_expr_t *ast_access(ast_expr_t *object, ast_expr_t *member);
ast_expr_t *ast_block(ast_expr_list_t *es);
ast_expr_t *ast_reserved_callable(ast_reserved_callable_type_t type, ast_expr_list_t *es);
ast_expr_t *ast_delete(ast_expr_t *ident);
ast_expr_t *ast_if_then(ast_expr_t *if_clause, ast_expr_t *then_clause);
ast_expr_t *ast_if_then_else(ast_expr_t *if_clause, ast_expr_t *then_clause, ast_expr_t *else_clause);
ast_expr_t *ast_do_while_loop(ast_expr_t *pred, ast_expr_t *cond);
ast_expr_t *ast_while_loop(ast_expr_t *cond, ast_expr_t *pred);
ast_expr_t *ast_for_loop(ast_expr_t *elem, ast_expr_t *iterable, ast_expr_t *pred);
ast_expr_t *ast_break(void);
ast_expr_t *ast_continue(void);
ast_expr_t *ast_empty(void);

#endif

