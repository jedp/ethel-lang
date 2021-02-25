#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/ast.h"
#include "../inc/lexer.h"
#include "../inc/parser.h"

ast_expr_t *parse_start(lexer_t *lexer);
ast_expr_list_t *parse_expr_list(lexer_t *lexer);
ast_expr_t *parse_expr(lexer_t *lexer);
ast_expr_t *parse_atom(lexer_t *lexer);
ast_expr_t *parse_eof(lexer_t *lexer);

bool is_binop(token_t *token) {
  return token->tag == TAG_PLUS
      || token->tag == TAG_MINUS
      || token->tag == TAG_TIMES
      || token->tag == TAG_DIVIDE
      || token->tag == TAG_AND
      || token->tag == TAG_OR
      || token->tag == TAG_MOD
      || token->tag == TAG_GT
      || token->tag == TAG_GE
      || token->tag == TAG_LT
      || token->tag == TAG_LE
      || token->tag == TAG_EQ;
}

uint8_t binop_preced(token_t *token) {
  switch (token->tag) {
    case TAG_TIMES:
    case TAG_DIVIDE:
    case TAG_MOD:
      return PRECED_MUL;
    case TAG_PLUS:
    case TAG_MINUS:
      return PRECED_ADD;
    case TAG_GT:
    case TAG_GE:
    case TAG_LT:
    case TAG_LE:
      return PRECED_GLT;
    case TAG_EQ:
      return PRECED_EQ;
    case TAG_AND:
      return PRECED_AND;
    case TAG_OR:
      return PRECED_OR;
    default:
      return PRECED_NONE;
      break;
  }
}

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

ast_expr_t *_parse_expr(lexer_t *lexer, int min_preced) {
  ast_expr_t *lhs = parse_atom(lexer);

  // The recursively recursive precedence-climbing algorithm.
  while (1) {
    token_t token = lexer->token;

    if (!is_binop(&token) ||
        binop_preced(&token) < min_preced) {
      goto done;
    }

    int preced = binop_preced(&token);

    // TODO the +1 is only for left-associative operators.
    int next_min_preced = preced +1;

    tag_t tag = token.tag;
    advance(lexer);

    switch(tag) {
      case TAG_PLUS:   lhs = ast_expr(AST_ADD, lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_MINUS:  lhs = ast_expr(AST_SUB, lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_TIMES:  lhs = ast_expr(AST_MUL, lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_DIVIDE: lhs = ast_expr(AST_DIV, lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_MOD:    lhs = ast_expr(AST_MOD, lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_AND:    lhs = ast_expr(AST_AND, lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_OR:     lhs = ast_expr(AST_OR,  lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_GT:     lhs = ast_expr(AST_GT,  lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_GE:     lhs = ast_expr(AST_GE,  lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_LT:     lhs = ast_expr(AST_LT,  lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_LE:     lhs = ast_expr(AST_LE,  lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_EQ:     lhs = ast_expr(AST_EQ,  lhs, _parse_expr(lexer, next_min_preced)); break;

      default:
        printf("what? why this %s?\n", tag_names[tag]);
        return lhs;
    }
  }

done:
  return lhs;
}

ast_expr_t *parse_expr(lexer_t *lexer) {
  return _parse_expr(lexer, PRECED_NONE);
}

/*
 * F -> int
 * F -> float
 * F -> ident
 * F -> ( E )
 * F -> if expr then expr [ else expr ]
 * F -> callable ( expr-list )
 */
ast_expr_t *parse_atom(lexer_t *lexer) {

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
    // TODO: Not actually right - doesn't do -(3+2), for example
    case TAG_MINUS: {
      advance(lexer);
      ast_expr_t *e = parse_atom(lexer);
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
      ast_expr_t *id = ast_ident(lexer->token.string);
      advance(lexer);
      if (lexer->token.tag == TAG_ASSIGN) {
        advance(lexer);
        return ast_assign(id, parse_expr(lexer));
      }
      return id;
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

