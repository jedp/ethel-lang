#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/ast.h"
#include "../inc/lexer.h"
#include "../inc/parser.h"

ast_expr_t *parse_start(lexer_t *lexer);
ast_expr_list_t *parse_expr_list(lexer_t *lexer);
ast_expr_t *parse_expr(lexer_t *lexer);
ast_expr_t *parse_term(lexer_t *lexer);
ast_expr_t *parse_factor(lexer_t *lexer);
ast_expr_t *parse_eof(lexer_t *lexer);

ast_reserved_callable_type_t ast_callable_type_for_tag(tag_t tag) {
  switch (tag) {
    case TAG_ABS: return AST_CALL_ABS;
    case TAG_SIN: return AST_CALL_SIN;
    case TAG_COS: return AST_CALL_COS;
    case TAG_TAN: return AST_CALL_TAN;
    case TAG_SQRT: return AST_CALL_SQRT;
    case TAG_EXP: return AST_CALL_EXP;
    case TAG_LN: return AST_CALL_LN;
    case TAG_LOG: return AST_CALL_LOG;
    default: return AST_CALL_UNDEFINED;
  }
}

ast_expr_t *parse_start(lexer_t *lexer) {
  return (lexer->token.tag == TAG_EOF) ? ast_empty() : parse_expr(lexer);
}

ast_expr_list_t *parse_expr_list(lexer_t *lexer) {
  ast_expr_list_t *node = malloc(sizeof(ast_expr_list_t));
  ast_expr_list_t *root = node;

  ast_expr_t *e = parse_expr(lexer);
  node->e = e;
  
  while (lexer->token.tag == TAG_COMMA) {
    advance(lexer);
    ast_expr_list_t *next = malloc(sizeof(ast_expr_list_t));
    e = parse_expr(lexer);
    next->e = e;

    node->next = next;
    node = next;
  }

  return root;
}

/*
 * E -> E + T
 * E -> E - T
 * E -> assign
 * E -> if E then E [ else E ]
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
    case TAG_ASSIGN: {
      advance(lexer);
      return ast_assign(e, parse_expr(lexer));
    }
    default: return e;
  }
}

/*
 * F -> int
 * F -> float
 * F -> ( E )
 * F -> callable ( expr-list )
 *
 * A factor produces a numeric value or an expression in parentheses.
 */
ast_expr_t *parse_factor(lexer_t *lexer) {

  switch (lexer->token.tag) {
    case TAG_NIL: {
      advance(lexer);
      return ast_nil();
    }
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
    case TAG_CHAR: {
      char c = lexer->token.ch;
      advance(lexer);
      return ast_char(c);
    }
    case TAG_STRING: {
      char* s = lexer->token.string;
      advance(lexer);
      return ast_string(s);
    }
    case TAG_LPAREN: {
      advance(lexer);
      ast_expr_t *e = parse_expr(lexer);
      if (!eat(lexer, TAG_RPAREN)) goto error;
      return e;
    }
    case TAG_IDENT: {
      advance(lexer);
      return ast_ident(lexer->token.string);
    }
    case TAG_IF: {
      advance(lexer);
      ast_expr_t *if_clause = parse_expr(lexer);
      if (!eat(lexer, TAG_THEN)) goto error;
      ast_expr_t *then_clause = parse_expr(lexer);
      if (lexer->token.tag == TAG_ELSE) {
        eat(lexer, TAG_ELSE);
        ast_expr_t *else_clause = parse_expr(lexer);
        return ast_if_then_else(if_clause, then_clause, else_clause);
      }
      return ast_if_then(if_clause, then_clause);
    }
    case TAG_ABS: 
    case TAG_SIN: 
    case TAG_COS: 
    case TAG_TAN: 
    case TAG_SQRT: 
    case TAG_EXP: 
    case TAG_LN: 
    case TAG_LOG: {
      int callable_type = ast_callable_type_for_tag(lexer->token.tag);
      advance(lexer);
      if (lexer->token.tag == TAG_LPAREN) {
        eat(lexer, TAG_LPAREN);
        ast_expr_list_t *es = parse_expr_list(lexer);
        if (!eat(lexer, TAG_RPAREN)) goto error;
        return ast_reserved_callable(callable_type, es);
      }
      return ast_ident(lexer->token.string);
    }
    default: goto error;
  }

error:
  printf("Expected int, float, or paren; got %s.\n", tag_names[lexer->token.tag]);
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

