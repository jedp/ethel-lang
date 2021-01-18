#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "calc.h"

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
  return (lexer->peek == _EOF) ? 0 : parse_expr(lexer);
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

  switch(lexer->peek) {
    case '+': advance(lexer);return parse_expr1(a + parse_term(lexer), lexer);
    case '-': advance(lexer);return parse_expr1(a - parse_term(lexer), lexer);
    default:return a;
   // default: goto error;
  }

error:
  printf("Syntax error at %d. Expected ADD or SUB; got %d.\n", lexer->pos, lexer->token.tag);
  lexer->err = lexer->pos;
  return 0;
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

  switch (lexer->peek) {
    case '*':advance(lexer); return parse_term1(a * parse_factor(lexer), lexer);
    case '/': advance(lexer);return parse_term1(a / parse_factor(lexer), lexer);
    default: return a;
   // default: goto error;
  }

error:
  printf("Syntax error at %d. Expected MUL or DIV.\n", lexer->pos);
  lexer->err = lexer->pos;
  return 0;
}

/*
 * F -> int
 * F -> float
 * F -> ( E )
 */
int parse_factor(lexer_t *lexer) {
  advance(lexer);
  if (lexer->err > -1) return 0;

  switch (lexer->token.tag) {
    case INT: return lexer->token.intval;
    case FLOAT: return (int) lexer->token.floatval;
    case LPAREN: {
      advance(lexer);
      int i = parse_expr(lexer);
      if (!eat(lexer, RPAREN)) goto error;
      return i;
    }
    default: goto error;
  }

error:
  printf("Expected int, float, or lparen.\n");
  lexer->err = lexer->pos;
  return 0;
}

