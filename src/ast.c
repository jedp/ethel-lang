#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include "../inc/ast.h"

ast_expr_t *ast_expr(ast_type_t type, ast_expr_t *e1, ast_expr_t *e2) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  node->type = type;
  node->e1 = e1;
  node->e2 = e2;
  return node;
}

ast_expr_t *ast_nil() {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  node->type = AST_NIL;
  return node;
}

ast_expr_t *ast_float(float value) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  node->type = AST_FLOAT;
  node->floatval = value;
  return node;
}

ast_expr_t *ast_int(int value) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  node->type = AST_INT;
  node->intval = value;
  return node;
}

ast_expr_t *ast_char(char c) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  node->type = AST_CHAR;
  node->charval = c;
  return node;
}

ast_expr_t *ast_string(char* s) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  char* stringval = malloc(sizeof(s) + 1);
  strcpy(stringval, s);
  node->type = AST_STRING;
  node->stringval = stringval;
  return node;
}

ast_expr_t *ast_ident(char* name) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  char* node_name = malloc(strlen(name) + 1);
  strcpy(node_name, name);
  node->type = AST_IDENT;
  node->stringval = node_name;
  return node;
}

ast_expr_t *ast_reserved_callable(ast_reserved_callable_type_t type, ast_expr_list_t *es) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  node->type = AST_RESERVED_CALLABLE;
  node->intval = type;
  node->e1 = es;
  return node;
}

ast_expr_t *ast_assign(ast_expr_t *ident, ast_expr_t *value) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  node->type = AST_ASSIGN;
  node->stringval = ident->stringval;
  node->e1 = ident;
  node->e2 = value;
  return node;
}

ast_expr_t *ast_if_then(ast_expr_t *if_clause, ast_expr_t *then_clause) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  node->type = AST_IF_THEN;
  node->e1 = if_clause;
  node->e2 = then_clause;
  return node;
}

ast_expr_t *ast_if_then_else(ast_expr_t *if_clause, ast_expr_t *then_clause, ast_expr_t *else_clause) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  node->type = AST_IF_THEN_ELSE;
  node->e1 = if_clause;
  node->e2 = then_clause;
  node->e3 = else_clause;
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
      printf(" %f", expr->floatval);
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

