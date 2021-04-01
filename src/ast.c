#include <stdlib.h>
#include <stdio.h>
#include "../inc/str.h"
#include "../inc/def.h"
#include "../inc/ast.h"

ast_expr_t *ast_node(ast_type_t type) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));

  switch(type) {
    case AST_INT:     node->intval = 0;      break;
    case AST_FLOAT:   node->floatval = 0.0;  break;
    case AST_CHAR:    node->charval = 0;     break;
    case AST_STRING:  node->stringval = "";  break;
    case AST_BOOLEAN: node->boolval = 0;     break;
    // Other nodes are not initialized.
    default: break;
  }

  node->type = type;
  node->flags = F_NONE;
  return node;
}

ast_expr_t *ast_empty() {
  ast_expr_t *node = ast_node(AST_EMPTY);
  return node;
}

ast_expr_t *ast_binop(ast_type_t type, ast_expr_t *a, ast_expr_t *b) {
  ast_expr_t *node = ast_node(type);
  switch(type) {
    case AST_ADD:
    case AST_SUB:
    case AST_MUL:
    case AST_DIV:
    case AST_MOD:
    case AST_AND:
    case AST_OR:
    case AST_GT:
    case AST_GE:
    case AST_LT:
    case AST_LE:
    case AST_EQ:
    case AST_NE:
      node->type = type;
      break;
    default:
      printf("Binop type %d unfamiliar\n", type);
      free(node);
      return ast_empty();
  }

  node->binop_args = malloc(sizeof(ast_binop_args_t));
  node->binop_args->a = a;
  node->binop_args->b = b;
  return node;
}

ast_expr_t *ast_cast(ast_expr_t *a, ast_expr_t *b) {
  ast_expr_t *node = ast_node(AST_CAST);
  node->cast_args = malloc(sizeof(ast_cast_args_t));
  node->cast_args->a = a;
  node->cast_args->b = b;
  return node;
}

ast_expr_t *ast_nil() {
  return ast_node(AST_NIL);
}

ast_expr_t *ast_list(char* type_name, ast_expr_list_t *nullable_init_es) {
  ast_expr_t *node = ast_node(AST_LIST);
  node->list = malloc(sizeof(ast_list_t));
  node->list->type_name = malloc(c_str_len(type_name) + 1);
  c_str_cp(node->list->type_name, type_name);
  if (nullable_init_es != NULL) {
    node->list->es = nullable_init_es;
  } else {
    node->list->es = NULL;
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

ast_expr_t *ast_char(char c) {
  ast_expr_t *node = ast_node(AST_CHAR);
  node->charval = c;
  return node;
}

ast_expr_t *ast_string(char* s) {
  ast_expr_t *node = ast_node(AST_STRING);
  char* stringval = malloc(c_str_len(s) + 1);
  c_str_cp(stringval, s);
  node->stringval = stringval;
  return node;
}

ast_expr_t *ast_boolean(boolean t) {
  ast_expr_t *node = ast_node(AST_BOOLEAN);
  node->boolval = t ? 1 : 0;
  return node;
}

ast_expr_t *ast_type(ast_type_t type) {
  ast_expr_t *node = ast_node(type);
  return node;
}

ast_expr_t *ast_ident(char* name) {
  ast_expr_t *node = ast_node(AST_IDENT);
  char* node_name = malloc(c_str_len(name) + 1);
  c_str_cp(node_name, name);
  node->stringval = node_name;
  return node;
}

ast_expr_t *ast_member_access(ast_expr_t *expr, char* member_name, ast_expr_list_t *args) {
  ast_expr_t *node = ast_node(AST_APPLY);

  node->application = malloc(sizeof(ast_apply_t));
  node->application->receiver = expr;

  node->application->member_name = malloc(c_str_len(member_name) + 1);
  c_str_cp(node->application->member_name, member_name);

  node->application->args = malloc(sizeof(ast_expr_list_t));
  node->application->args = args;

  return node;
}

ast_expr_t *ast_type_name(char* name) {
  ast_expr_t *node = ast_node(AST_TYPE_NAME);
  char* node_name = malloc(c_str_len(name) + 1);
  c_str_cp(node_name, name);
  node->stringval = node_name;
  return node;
}

ast_expr_t *ast_range(ast_expr_t *from, ast_expr_t *to) {
  ast_expr_t *node = ast_node(AST_RANGE);
  node->range = malloc(sizeof(ast_range_args_t));
  node->range->from = from;
  node->range->to = to;
  return node;
}

ast_expr_t *ast_method(char* name, ast_expr_list_t *args) {
  ast_expr_t *node = ast_node(AST_METHOD);
  node->method = malloc(sizeof(ast_method_t));
  node->method->name = malloc(c_str_len(name) + 1);
  c_str_cp(node->method->name, name);
  node->method->args = args;
  return node;
}

ast_expr_t *ast_access(ast_expr_t *object, ast_expr_t *member) {
  if (member->type != AST_METHOD) {
    printf("Method access only. Not allowed: '%s'\n", ast_node_names[member->type]);
    return ast_empty();
  }

  return ast_member_access(object, member->method->name, member->method->args);
}

ast_expr_t *ast_block(ast_expr_list_t *es) {
  ast_expr_t *node = ast_node(AST_BLOCK);
  node->block_exprs = malloc(sizeof(ast_expr_list_t));
  node->block_exprs = es;
  return node;
}

ast_expr_t *ast_reserved_callable(ast_reserved_callable_type_t type, ast_expr_list_t *es) {
  ast_expr_t *node = ast_node(AST_RESERVED_CALLABLE);
  node->reserved_callable = malloc(sizeof(ast_reserved_callable_t));
  node->reserved_callable->type = type;
  node->reserved_callable->es = malloc(sizeof(ast_expr_list_t));
  node->reserved_callable->es = es;
  return node;
}

ast_expr_t *ast_assign_val(ast_expr_t *ident, ast_expr_t *value) {
  ast_expr_t *node = ast_node(AST_ASSIGN);
  node->stringval = ident->stringval;
  node->flags = F_NONE;
  node->assignment = malloc(sizeof(ast_assign_t));
  node->assignment->ident = ident;
  node->assignment->value = value;
  return node;
}

ast_expr_t *ast_assign_var(ast_expr_t *ident, ast_expr_t *value) {
  ast_expr_t *node = ast_node(AST_ASSIGN);
  node->stringval = ident->stringval;
  node->flags = F_VAR;
  node->assignment = malloc(sizeof(ast_assign_t));
  node->assignment->ident = ident;
  node->assignment->value = value;
  return node;
}

ast_expr_t *ast_reassign(ast_expr_t *ident, ast_expr_t *value) {
  ast_expr_t *node = ast_node(AST_REASSIGN);
  node->stringval = ident->stringval;
  node->assignment = malloc(sizeof(ast_assign_t));
  node->assignment->ident = ident;
  node->assignment->value = value;
  return node;
}

ast_expr_t *ast_delete(ast_expr_t *ident) {
  ast_expr_t *node = ast_node(AST_DELETE);
  node->stringval = ident->stringval;
  return node;
}

ast_expr_t *ast_if_then(ast_expr_t *if_clause, ast_expr_t *then_clause) {
  ast_expr_t *node = ast_node(AST_IF_THEN);
  node->if_then_args = malloc(sizeof(ast_if_then_args_t));
  node->if_then_args->cond = if_clause;
  node->if_then_args->pred = then_clause;
  return node;
}

ast_expr_t *ast_if_then_else(ast_expr_t *if_clause, ast_expr_t *then_clause, ast_expr_t *else_clause) {
  ast_expr_t *node = ast_node(AST_IF_THEN_ELSE);
  node->if_then_else_args = malloc(sizeof(ast_if_then_else_args_t));
  node->if_then_else_args->cond = if_clause;
  node->if_then_else_args->pred = then_clause;
  node->if_then_else_args->else_pred = else_clause;
  return node;
}

ast_expr_t *ast_while_loop(ast_expr_t *cond, ast_expr_t *pred) {
  ast_expr_t *node = ast_node(AST_WHILE_LOOP);
  node->while_loop = malloc(sizeof(ast_while_loop_t));
  node->while_loop->cond = cond;
  node->while_loop->pred = pred;
  return node;
}

ast_expr_t *ast_for_loop(ast_expr_t *index,
                         ast_expr_t *range,
                         ast_expr_t *pred) {
  ast_expr_t *node = ast_node(AST_FOR_LOOP);
  node->for_loop = malloc(sizeof(ast_for_loop_t));
  node->for_loop->index = index;
  node->for_loop->range = range;
  node->for_loop->pred = pred;
  return node;
}

void _pretty_print(ast_expr_t *expr, int indent) {
  for (int i = 0; i < indent * 2; i++) {
    printf(" ");
  }

  printf("(%s", ast_node_names[expr->type]);
  switch(expr->type) {
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
      _pretty_print(expr->binop_args->a, indent + 1);
      _pretty_print(expr->binop_args->b, indent + 1);
      for (int i = 0; i < indent * 2; i++) {
        printf(" ");
      }
      break;
    default:
      printf(" <something> ");
      break;
  }

  printf(")\n");
}

void pretty_print(ast_expr_t *expr) {
  _pretty_print(expr, 0);
}

