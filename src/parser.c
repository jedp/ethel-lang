#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/ast.h"
#include "../inc/lexer.h"
#include "../inc/parser.h"

ast_expr_t *parse_start(lexer_t *lexer);
ast_expr_t *parse_expr(lexer_t *lexer);
ast_expr_t *parse_term(lexer_t *lexer);
ast_expr_t *parse_factor(lexer_t *lexer);
ast_expr_t *parse_eof(lexer_t *lexer);

ast_expr_t *parse_start(lexer_t *lexer) {
  return (lexer->token.tag == TAG_EOF) ? ast_empty() : parse_expr(lexer);
}

/*
 * E -> E + T
 * E -> E - T
 * E -> E
 *
 * An expression produces a term and maybe an additive or subractive expression.
 */
ast_expr_t *parse_expr(lexer_t *lexer) {
  ast_expr_t *e = parse_term(lexer);

  switch(lexer->token.tag) {
    case TAG_PLUS: {
      advance(lexer);
      return ast_expr(AST_ADD, e, parse_expr(lexer));
    }
    case TAG_MINUS: {
      advance(lexer);
      return ast_expr(AST_SUB, e, parse_expr(lexer));
    }
    default: return e;
  }
}

/*
 * T -> F * T
 * T -> F / T
 * T -> F
 *
 * A term produces a factor and maybe a multiplicative term.
 */
ast_expr_t *parse_term(lexer_t *lexer) {
  ast_expr_t *e = parse_factor(lexer);

  switch (lexer->token.tag) {
    case TAG_TIMES: {
      advance(lexer);
      return ast_expr(AST_MUL, e, parse_term(lexer));
    }
    case TAG_DIVIDE: {
      advance(lexer);
      return ast_expr(AST_DIV, e, parse_term(lexer));
    }
    default: return e;
  }
}

/*
 * F -> int
 * F -> float
 * F -> ( E )
 *
 * A factor produces a numeric value or an expression in parentheses.
 */
ast_expr_t *parse_factor(lexer_t *lexer) {

  switch (lexer->token.tag) {
    case TAG_INT: {
      int i = lexer->token.intval;
      advance(lexer);
      return ast_int(i);
    }
    case TAG_FLOAT: {
      float f = lexer->token.floatval;
      advance(lexer);
      return ast_float(f);
    }
    case TAG_MINUS: {
      advance(lexer);
      ast_expr_t *e = parse_factor(lexer);
      if (e->type == AST_INT) e->intval *= -1;
      if (e->type == AST_FLOAT) e->intval *= -1;
      return e;
    }
    case TAG_LPAREN: {
      advance(lexer);
      ast_expr_t *e = parse_expr(lexer);
      if (!eat(lexer, TAG_RPAREN)) goto error;
      return e;
    }
    default: goto error;
  }

error:
  printf("Expected int, float, or paren; got tag %d.\n", lexer->token.tag);
  lexer->err = lexer->pos;
  return ast_empty();
}

ast_expr_t *parse_program(char *input) {
  lexer_t lexer;
  lexer_init(&lexer, input, strlen(input));

  ast_expr_t *ast = parse_start(&lexer);
  if (lexer.err > -1) {
    printf("Syntax error at %d\n", lexer.pos);
    return ast_empty();
  }

  return ast;
}

