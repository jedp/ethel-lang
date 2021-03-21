#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>
#include <stdio.h>
#include "../inc/def.h"
#include "../inc/ast.h"

ast_expr_t *ast_node(ast_type_t type) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));

  switch(type) {
    case AST_INT:     node->intval = 0;      break;
    case AST_FLOAT:   node->floatval = 0.0;  break;
    case AST_CHAR:    node->charval = 0;     break;
    case AST_STRING:  node->stringval = "";  break;
    case AST_BOOLEAN: node->intval = 0;      break;
    // Other nodes are not initialized.
    default: break;
  }

  node->type = type;
  node->flags = F_NONE;
  node->e1 = NULL;
  node->e2 = NULL;
  node->e3 = NULL;
  node->e4 = NULL;
  return node;
}

ast_expr_t *ast_expr(ast_type_t type, ast_expr_t *e1, ast_expr_t *e2) {
  ast_expr_t *node = ast_node(type);
  node->e1 = e1;
  node->e2 = e2;
  return node;
}

ast_expr_t *ast_cast(ast_expr_t *e1, ast_expr_t *e2) {
  ast_expr_t *node = ast_node(AST_CAST);
  node->e1 = e1;
  node->e2 = e2;
  return node;
}

ast_expr_t *ast_nil() {
  return ast_node(AST_NIL);
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
  char* stringval = malloc(strlen(s) + 1);
  strcpy(stringval, s);
  node->stringval = stringval;
  return node;
}

ast_expr_t *ast_boolean(bool t) {
  ast_expr_t *node = ast_node(AST_BOOLEAN);
  node->intval = t ? 1 : 0;
  return node;
}

ast_expr_t *ast_type(ast_type_t type) {
  ast_expr_t *node = ast_node(type);
  return node;
}

ast_expr_t *ast_ident(char* name) {
  ast_expr_t *node = ast_node(AST_IDENT);
  char* node_name = malloc(strlen(name) + 1);
  strcpy(node_name, name);
  node->stringval = node_name;
  return node;
}

ast_expr_t *ast_block(ast_expr_list_t *es) {
  ast_expr_t *node = ast_node(AST_BLOCK);
  node->e1 = es;
  return node;
}

ast_expr_t *ast_reserved_callable(ast_reserved_callable_type_t type, ast_expr_list_t *es) {
  ast_expr_t *node = ast_node(AST_RESERVED_CALLABLE);
  node->intval = (int) type;
  node->e1 = es;
  return node;
}

ast_expr_t *ast_assign(ast_expr_t *ident, ast_expr_t *value, uint8_t flags) {
  ast_expr_t *node = ast_node(AST_ASSIGN);
  node->stringval = ident->stringval;
  node->flags = flags;
  node->e1 = ident;
  node->e2 = value;
  return node;
}

ast_expr_t *ast_delete(ast_expr_t *ident) {
  ast_expr_t *node = ast_node(AST_DELETE);
  node->stringval = ident->stringval;
  return node;
}

ast_expr_t *ast_if_then(ast_expr_t *if_clause, ast_expr_t *then_clause) {
  ast_expr_t *node = ast_node(AST_IF_THEN);
  node->e1 = if_clause;
  node->e2 = then_clause;
  return node;
}

ast_expr_t *ast_if_then_else(ast_expr_t *if_clause, ast_expr_t *then_clause, ast_expr_t *else_clause) {
  ast_expr_t *node = ast_node(AST_IF_THEN_ELSE);
  node->e1 = if_clause;
  node->e2 = then_clause;
  node->e3 = else_clause;
  return node;
}

ast_expr_t *ast_while_loop(ast_expr_t *cond, ast_expr_t *pred) {
  ast_expr_t *node = ast_node(AST_WHILE_LOOP);
  node->e1 = cond;
  node->e2 = pred;
  return node;
}

ast_expr_t *ast_for_loop(ast_expr_t *index,
                         ast_expr_t *start,
                         ast_expr_t *end,
                         ast_expr_t *pred) {
  ast_expr_t *node = ast_node(AST_FOR_LOOP);
  node->e1 = index;
  node->e2 = start;
  node->e3 = end;
  node->e4 = pred;
  return node;
}

ast_expr_t *ast_empty() {
  return ast_expr(AST_EMPTY, 0, 0);
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
      _pretty_print(expr->e1, indent + 1);
      _pretty_print(expr->e2, indent + 1);
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

