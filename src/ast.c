#include <assert.h>
#include <stdio.h>
#include "../inc/type.h"
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/def.h"
#include "../inc/ast.h"

ast_expr_t *ast_node(type_t type) {
  assert(type > TYPE_ERR_DO_NOT_USE);
  ast_expr_t *node = (ast_expr_t*) alloc_type(type, F_NONE);

  switch(type) {
    case AST_INT:     node->intval = 0;      break;
    case AST_FLOAT:   node->floatval = 0.0;  break;
    case AST_BYTE:    node->byteval = 0;     break;
    case AST_BOOLEAN: node->boolval = 0;     break;
    case AST_BYTEARRAY:
    case AST_STRING: {
                      node->bytearray = bytearray_alloc(0);
                      break;
                     }
    // Other nodes are not initialized.
    default: break;
  }

  return node;
}

ast_expr_t *ast_empty(void) {
  ast_expr_t *node = ast_node(AST_EMPTY);
  return node;
}

ast_expr_t *ast_unary(type_t type, ast_expr_t *a) {
  ast_expr_t *node = ast_node(type);
  switch(type) {
    case AST_NEGATE:
    case AST_NOT:
    case AST_BITWISE_NOT:
      // Familiar.
      break;
    default:
      printf("Unary type %d unfamiliar\n", type);
      mem_free(node);
      node = NULL;
      return ast_empty();
  }

  node->unary_arg = (ast_unary_arg_t*) alloc_type(AST_UNARY_ARG, F_NONE);

  node->unary_arg->a = a;
  return node;
}

ast_expr_t *ast_op(type_t type, ast_expr_t *a, ast_expr_t *b) {
  ast_expr_t *node = ast_node(type);
  switch(type) {
    case AST_ADD:
    case AST_SUB:
    case AST_MUL:
    case AST_DIV:
    case AST_MOD:
    case AST_AND:
    case AST_OR:
    case AST_BITWISE_OR:
    case AST_BITWISE_XOR:
    case AST_BITWISE_AND:
    case AST_BITWISE_SHL:
    case AST_BITWISE_SHR:
    case AST_GT:
    case AST_GE:
    case AST_LT:
    case AST_LE:
    case AST_EQ:
    case AST_NE:
    case AST_IS:
    case AST_IN:
    case AST_SUBSCRIPT:
    case AST_MAPS_TO:
    case AST_ASSIGN:
      // Familiar.
      break;
    default:
      printf("Op %s unfamiliar\n", type_names[type]);
      mem_free(node);
      node = NULL;
      return ast_empty();
  }

  node->op_args = (ast_op_args_t*) alloc_type(AST_BINOP_ARGS, F_NONE);

  node->op_args->a = a;
  node->op_args->b = b;
  return node;
}

ast_expr_t *ast_cast(ast_expr_t *a, ast_expr_t *b) {
  ast_expr_t *node = ast_node(AST_CAST);
  node->cast_args = (ast_cast_args_t*) alloc_type(AST_CAST_ARGS, F_NONE);

  node->cast_args->a = a;
  node->cast_args->b = b;
  return node;
}

ast_expr_t *ast_nil(void) {
  return ast_node(AST_NIL);
}

ast_expr_t *ast_list(ast_expr_list_t *nullable_init_es) {
  ast_expr_t *node = ast_node(AST_LIST);
  gc_header_t *hdr = (gc_header_t*) node;
  hdr->flags |= F_ENV_ASSIGNABLE;
  node->list = (ast_list_t*) alloc_type(AST_LIST_ELEMS, F_NONE);

  if (nullable_init_es != NULL) {
    node->list->es = nullable_init_es;
  } else {
    node->list->es = NULL;
  }
  return node;
}

ast_expr_t *ast_dict(ast_expr_kv_list_t *nullable_kvs) {
  ast_expr_t *node = ast_node(AST_DICT);
  gc_header_t *hdr = (gc_header_t*) node;
  hdr->flags |= F_ENV_ASSIGNABLE;

  node->dict = (ast_dict_t*) alloc_type(AST_DICT_KVS, F_NONE);
  node->dict->kv = (ast_expr_kv_list_t*) alloc_type(AST_DICT_KV, F_NONE);

  if (nullable_kvs != NULL) {
    node->dict->kv = nullable_kvs;
  } else {
    node->dict->kv = NULL;
  }
  return node;
}

ast_expr_t *ast_float(float value) {
  ast_expr_t *node = ast_node(AST_FLOAT);
  node->floatval = value;
  return node;
}

ast_expr_t *ast_int(int value) {
  ast_expr_t *node = ast_node(AST_INT);
  node->intval = value;
  return node;
}

ast_expr_t *ast_byte(byte b) {
  ast_expr_t *node = ast_node(AST_BYTE);
  node->byteval = b;
  return node;
}

ast_expr_t *ast_array_decl(ast_expr_t *size) {
  ast_expr_t *node = ast_node(AST_BYTEARRAY_DECL);
  node->array_decl = (ast_array_decl_t*) alloc_type(AST_BYTEARRAY_DECL_DATA, F_NONE);
  node->array_decl->size = size;
  return node;
}

ast_expr_t *ast_string(bytearray_t *s) {
  ast_expr_t *node = ast_node(AST_STRING);
  node->bytearray = bytearray_clone(s);
  return node;
}

ast_expr_t *ast_boolean(boolean t) {
  ast_expr_t *node = ast_node(AST_BOOLEAN);
  node->boolval = t ? 1 : 0;
  return node;
}

ast_expr_t *ast_type(type_t type) {
  ast_expr_t *node = ast_node(type);
  return node;
}

ast_expr_t *ast_ident(bytearray_t *name) {
  ast_expr_t *node = ast_node(AST_IDENT);
  gc_header_t *hdr = (gc_header_t*) node;
  hdr->flags |= F_ENV_ASSIGNABLE;
  node->bytearray = bytearray_clone(name);
  return node;
}

ast_expr_t *ast_ident_decl(bytearray_t *name, flags_t flags) {
  ast_expr_t *node = ast_ident(name);
  gc_header_t *hdr = (gc_header_t*) node;
  hdr->flags = flags | F_ENV_ASSIGNABLE;
  return node;
}

ast_expr_t *ast_member_access(ast_expr_t *expr,
                              bytearray_t *member_name,
                              ast_expr_list_t *args) {
  ast_expr_t *node = ast_node(AST_APPLY);

  node->application = (ast_apply_t*) alloc_type(AST_APPLY_DATA, F_NONE);
  node->application->receiver = expr;
  node->application->member_name = bytearray_clone(member_name);

  node->application->args = (ast_expr_list_t*) alloc_type(AST_EXPR_LIST, F_NONE);
  node->application->args = args;

  return node;
}

ast_expr_t *ast_type_name(bytearray_t *name) {
  ast_expr_t *node = ast_node(AST_TYPE_NAME);
  node->bytearray = name;
  return node;
}

ast_expr_t *ast_range(ast_expr_t *from, ast_expr_t *to) {
  ast_expr_t *node = ast_node(AST_RANGE);
  node->range = (ast_range_args_t*) alloc_type(AST_RANGE_ARGS, F_NONE);
  node->range->from = from;
  node->range->to = to;
  node->range->step = NULL;
  return node;
}

ast_expr_t *ast_range_step(ast_expr_t *expr, ast_expr_t *step) {
  if (TYPEOF(expr) != AST_RANGE) {
    printf("left operand to step not a range\n");
    return ast_nil();
  }
  expr->range->step = step;
  return expr;
}

ast_expr_t *ast_method_call(bytearray_t *name, ast_expr_list_t *args) {
  ast_expr_t *node = ast_node(AST_METHOD_CALL);
  node->method_call = (ast_method_t*) alloc_type(AST_METHOD_CALL_DATA, F_NONE);
  node->method_call->name = bytearray_clone(name);
  node->method_call->args = args;
  return node;
}

ast_expr_t *ast_access(ast_expr_t *object, ast_expr_t *member) {
  if (TYPEOF(member) != AST_METHOD_CALL) {
    printf("Method access only. Not allowed: '%s'\n", type_names[TYPEOF(member)]);
    return ast_empty();
  }

  return ast_member_access(object, member->method_call->name, member->method_call->args);
}

ast_expr_t *ast_block(ast_expr_list_t *es) {
  ast_expr_t *node = ast_node(AST_BLOCK);
  node->block_exprs = es;
  return node;
}

ast_expr_t *ast_func_def(ast_fn_arg_decl_t *argnames,
                         ast_expr_list_t *es) {
  ast_expr_t *node = ast_node(AST_FUNCTION_DEF);
  node->func_def = (ast_func_def_t*) alloc_type(AST_FUNCTION_DEF_DATA, F_NONE);
  node->func_def->argnames = argnames;
  node->func_def->block_exprs = es;
  return node;
}

ast_expr_t *ast_func_call(ast_expr_t *expr, ast_expr_list_t *args) {
  ast_expr_t *node = ast_node(AST_FUNCTION_CALL);
  node->func_call = (ast_func_call_t*) alloc_type(AST_FUNCTION_CALL_DATA, F_NONE);
  node->func_call->expr = expr;
  node->func_call->args = args;
  return node;
}

ast_expr_t *ast_func_return(ast_expr_list_t *es) {
  ast_expr_t *node = ast_node(AST_FUNCTION_RETURN);
  node->func_return_values = es;
  return node;
}

ast_expr_t *ast_reserved_callable(ast_reserved_callable_type_t type, ast_expr_list_t *es) {
  ast_expr_t *node = ast_node(AST_RESERVED_CALLABLE);
  node->reserved_callable = (ast_reserved_callable_t*) alloc_type(
      AST_RESERVED_CALLABLE_DATA, F_NONE);
  node->reserved_callable->type = type;

  node->reserved_callable->es = (ast_expr_list_t*) alloc_type(
      AST_EXPR_LIST, F_NONE);
  node->reserved_callable->es = es;
  return node;
}

ast_expr_t *ast_delete(ast_expr_t *ident) {
  ast_expr_t *node = ast_node(AST_DELETE);
  node->bytearray = ident->bytearray;
  return node;
}

ast_expr_t *ast_if_then(ast_expr_t *if_clause, ast_expr_t *then_clause) {
  ast_expr_t *node = ast_node(AST_IF_THEN);
  node->if_then_args = (ast_if_then_args_t*) alloc_type(
      AST_IF_THEN_DATA, F_NONE);
  node->if_then_args->cond = if_clause;
  node->if_then_args->pred = then_clause;
  return node;
}

ast_expr_t *ast_if_then_else(ast_expr_t *if_clause, ast_expr_t *then_clause, ast_expr_t *else_clause) {
  ast_expr_t *node = ast_node(AST_IF_THEN_ELSE);
  node->if_then_else_args = (ast_if_then_else_args_t*) alloc_type(
      AST_IF_THEN_ELSE_DATA, F_NONE);
  node->if_then_else_args->cond = if_clause;
  node->if_then_else_args->pred = then_clause;
  node->if_then_else_args->else_pred = else_clause;
  return node;
}

ast_expr_t *ast_do_while_loop(ast_expr_t *pred, ast_expr_t *cond) {
  ast_expr_t *node = ast_node(AST_DO_WHILE_LOOP);
  node->do_while_loop = (ast_do_while_loop_t*) alloc_type(
      AST_DO_WHILE_LOOP_DATA, F_NONE);
  node->do_while_loop->pred = pred;
  node->do_while_loop->cond = cond;
  return node;
}

ast_expr_t *ast_while_loop(ast_expr_t *cond, ast_expr_t *pred) {
  ast_expr_t *node = ast_node(AST_WHILE_LOOP);
  node->while_loop = (ast_while_loop_t*) alloc_type(AST_WHILE_LOOP_DATA, F_NONE);
  node->while_loop->cond = cond;
  node->while_loop->pred = pred;
  return node;
}

ast_expr_t *ast_for_loop(ast_expr_t *elem,
                         ast_expr_t *iterable,
                         ast_expr_t *pred) {
  ast_expr_t *node = ast_node(AST_FOR_LOOP);
  node->for_loop = (ast_for_loop_t*) alloc_type(AST_FOR_LOOP_DATA, F_NONE);
  node->for_loop->elem = elem;
  node->for_loop->iterable = iterable;
  node->for_loop->pred = pred;
  return node;
}

ast_expr_t *ast_break(void) {
  return ast_node(AST_BREAK);
}

ast_expr_t *ast_continue(void) {
  return ast_node(AST_CONTINUE);
}

void _pretty_print(ast_expr_t *expr, int indent) {
  for (int i = 0; i < indent * 2; i++) {
    printf(" ");
  }

  int children = ((gc_header_t*) expr)->children;

  printf("(%s", type_names[TYPEOF(expr)]);

  switch (TYPEOF(expr)) {
    case AST_INT:     printf(" %d)\n", expr->intval);               break;
    case AST_FLOAT:   printf(" %f)\n", expr->floatval);             break;
    case AST_BYTE:    printf(" '%c')\n", expr->byteval);            break;
    case AST_BOOLEAN: printf(" %c)\n", expr->boolval ? 'T' : 'F');  break;
    case TYPE_BYTEARRAY_DATA:
        printf(" \"%s\")\n", bytearray_to_c_str((bytearray_t*) expr));
        break;
    default:          printf(")\n");                                break;
  }

  for (int i = 0; i < children; ++i) {
    size_t offset = sizeof(gc_header_t) + (i * sizeof(void*));
    void **child = (void*) (expr) + offset;
    if (*child != NULL) _pretty_print((ast_expr_t*) *child, indent + 1);
  }
}

void pretty_print(ast_expr_t *expr) {
  _pretty_print(expr, 0);
}

