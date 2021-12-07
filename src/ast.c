#include <stdio.h>
#include "../inc/type.h"
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/def.h"
#include "../inc/ast.h"

ast_expr_t *ast_node(type_t type) {
  ast_expr_t *node = mem_alloc(sizeof(ast_expr_t));

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

  mark_traceable(node, type, F_NONE);
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

  node->unary_arg = mem_alloc(sizeof(ast_unary_arg_t));

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
    case AST_BITWISE_NOT:
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

  node->op_args = mem_alloc(sizeof(ast_op_args_t));
  node->op_args->a = a;
  node->op_args->b = b;
  return node;
}

ast_expr_t *ast_cast(ast_expr_t *a, ast_expr_t *b) {
  ast_expr_t *node = ast_node(AST_CAST);
  node->cast_args = mem_alloc(sizeof(ast_cast_args_t));
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
  node->list = mem_alloc(sizeof(ast_list_t));
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
  node->dict = mem_alloc(sizeof(ast_dict_t));
  node->dict->kv = mem_alloc(sizeof(ast_expr_kv_list_t));
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
  node->array_decl = mem_alloc(sizeof(ast_array_decl_t));
  node->array_decl->size = size;
  return node;
}

ast_expr_t *ast_string(bytearray_t *s) {
  ast_expr_t *node = ast_node(AST_STRING);
  node->bytearray = bytearray_clone(s);
  mark_traceable(node->bytearray, TYPE_BYTEARRAY, F_NONE);
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
  node->bytearray = name;
  mark_traceable(node->bytearray, TYPE_BYTEARRAY, F_NONE);
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

  node->application = mem_alloc(sizeof(ast_apply_t));
  node->application->receiver = expr;

  node->application->member_name = bytearray_clone(member_name);

  node->application->args = mem_alloc(sizeof(ast_expr_list_t));
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
  node->range = mem_alloc(sizeof(ast_range_args_t));
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
  node->method_call = mem_alloc(sizeof(ast_method_t));
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
  node->block_exprs = mem_alloc(sizeof(ast_expr_list_t));
  node->block_exprs = es;
  return node;
}

ast_expr_t *ast_func_def(ast_fn_arg_decl_t *argnames,
                       ast_expr_list_t *es) {
  ast_expr_t *node = ast_node(AST_FUNCTION_DEF);
  node->func_def = mem_alloc(sizeof(ast_func_def_t));
  node->func_def->argnames = argnames;
  node->func_def->block_exprs = es;
  return node;
}

ast_expr_t *ast_func_call(ast_expr_t *expr, ast_expr_list_t *args) {
  ast_expr_t *node = ast_node(AST_FUNCTION_CALL);
  node->func_call = mem_alloc(sizeof(ast_func_call_t));
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
  node->reserved_callable = mem_alloc(sizeof(ast_reserved_callable_t));
  node->reserved_callable->type = type;
  node->reserved_callable->es = mem_alloc(sizeof(ast_expr_list_t));
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
  node->if_then_args = mem_alloc(sizeof(ast_if_then_args_t));
  node->if_then_args->cond = if_clause;
  node->if_then_args->pred = then_clause;
  return node;
}

ast_expr_t *ast_if_then_else(ast_expr_t *if_clause, ast_expr_t *then_clause, ast_expr_t *else_clause) {
  ast_expr_t *node = ast_node(AST_IF_THEN_ELSE);
  node->if_then_else_args = mem_alloc(sizeof(ast_if_then_else_args_t));
  node->if_then_else_args->cond = if_clause;
  node->if_then_else_args->pred = then_clause;
  node->if_then_else_args->else_pred = else_clause;
  return node;
}

ast_expr_t *ast_do_while_loop(ast_expr_t *pred, ast_expr_t *cond) {
  ast_expr_t *node = ast_node(AST_DO_WHILE_LOOP);
  node->do_while_loop = mem_alloc(sizeof(ast_do_while_loop_t));
  node->do_while_loop->pred = pred;
  node->do_while_loop->cond = cond;
  return node;
}

ast_expr_t *ast_while_loop(ast_expr_t *cond, ast_expr_t *pred) {
  ast_expr_t *node = ast_node(AST_WHILE_LOOP);
  node->while_loop = mem_alloc(sizeof(ast_while_loop_t));
  node->while_loop->cond = cond;
  node->while_loop->pred = pred;
  return node;
}

ast_expr_t *ast_for_loop(ast_expr_t *elem,
                         ast_expr_t *iterable,
                         ast_expr_t *pred) {
  ast_expr_t *node = ast_node(AST_FOR_LOOP);
  node->for_loop = mem_alloc(sizeof(ast_for_loop_t));
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

  printf("(%s", type_names[TYPEOF(expr)]);
  switch(TYPEOF(expr)) {
    case AST_INT:
      printf(" %d", expr->intval);
      break;
    case AST_FLOAT:
      printf(" %f", (double) expr->floatval);
      break;
    case AST_ADD:
    case AST_SUB:
    case AST_MUL:
    case AST_DIV:
      printf("\n");
      _pretty_print(expr->op_args->a, indent + 1);
      _pretty_print(expr->op_args->b, indent + 1);
      for (int i = 0; i < indent * 2; i++) {
        printf(" ");
      }
      break;
    default:
      printf(" <%s> ", type_names[TYPEOF(expr)]);
      break;
  }

  printf(")\n");
}

void pretty_print(ast_expr_t *expr) {
  _pretty_print(expr, 0);
}

