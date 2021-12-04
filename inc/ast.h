#ifndef _AST_H
#define _AST_H

#include <inttypes.h>
#include "def.h"

typedef uint8_t ast_type_t;

typedef uint8_t ast_reserved_callable_type_t;

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
  ast_expr_t *step;
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
ast_expr_t *ast_range_step(ast_expr_t *range, ast_expr_t *step);
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

