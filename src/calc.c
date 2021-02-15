#include <stdio.h>
#include <string.h>
#include "../inc/lexer.h"
#include "../inc/calc.h"

/**********************************************************************
 * Recursive-descent interpreter for arithmetic expressions.
 *
 * Grammar:
 *
 * S -> E $
 *
 * E -> T E'
 *
 * E' -> + T E'
 * E' -> - T E'
 * E' ->
 *
 * T -> F T'
 *
 * T' -> * F T'
 * T' -> / F T'
 * T' ->
 *
 * F -> id
 * F -> num
 * F -> ( E )
 **********************************************************************/

int parse_expr(lexer_t *lexer);
int parse_expr1(int a, lexer_t *lexer);
int parse_term(lexer_t *lexer);
int parse_term1(int a, lexer_t *lexer);
int parse_factor(lexer_t *lexer);
int parse_eof(lexer_t *lexer);

/* S -> E $ */
int parse_start(lexer_t *lexer) {
  return (lexer->token.tag == TAG_EOF) ? 0 : parse_expr(lexer);
}

/* E -> T E' */
int parse_expr(lexer_t *lexer) {
  int t = parse_term(lexer);
  return parse_expr1(t, lexer);
}

/*
 * E' -> + T E'
 * E' -> - T E'
 * E' ->
 */
int parse_expr1(int a, lexer_t *lexer) {
  if (lexer->err > -1) return 0;

  switch(lexer->token.tag) {
    case TAG_PLUS: {
      advance(lexer);
      return parse_expr1(a + parse_term(lexer), lexer);
    }
    case TAG_MINUS: {
      advance(lexer);
      return parse_expr1(a - parse_term(lexer), lexer);
    }
    default:return a;
  }
}

/* T -> F T' */
int parse_term(lexer_t *lexer) {
  int f = parse_factor(lexer);
  return parse_term1(f, lexer);
}

/*
 * T' -> * F T'
 * T' -> / F T'
 * T' ->
 */
int parse_term1(int a, lexer_t *lexer) {
  if (lexer->err > -1) return 0;

  switch (lexer->token.tag) {
    case TAG_TIMES: {
      advance(lexer);
      return parse_term1(a * parse_factor(lexer), lexer);
    }
    case TAG_DIVIDE: {
      advance(lexer);
      return parse_term1(a / parse_factor(lexer), lexer);
    }
    default: return a;
  }
}

/*
 * F -> int
 * F -> float
 * F -> ( E )
 */
int parse_factor(lexer_t *lexer) {
  if (lexer->err > -1) return 0;

  switch (lexer->token.tag) {
    case TAG_INT: {
      int i = lexer->token.intval;
      advance(lexer);
      return i;
    }
    case TAG_FLOAT: {
      float f = lexer->token.floatval;
      advance(lexer);
      return (int) f;
    }
    case TAG_MINUS: {
      advance(lexer);
      return -1 * parse_factor(lexer);
    }
    case TAG_LPAREN: {
      advance(lexer);
      int i = parse_expr(lexer);
      if (!eat(lexer, TAG_RPAREN)) goto error;
      return i;
    }
    default: goto error;
  }

error:
  printf("Expected int, float, or paren; got tag %d.\n", lexer->token.tag);
  lexer->err = lexer->pos;
  return 0;
}

