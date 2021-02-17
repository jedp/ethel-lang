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

ast_expr_t *ast_ident(char* name) {
  ast_expr_t *node = malloc(sizeof(ast_expr_t));
  char* node_name = malloc(strlen(name) + 1);
  strcpy(node_name, name);
  node->type = AST_IDENT;
  node->stringval = node_name;
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

