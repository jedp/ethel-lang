#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/def.h"
#include "../inc/ast.h"
#include "../inc/lexer.h"
#include "../inc/parser.h"

ast_expr_t *parse_start(lexer_t *lexer);
ast_expr_list_t *parse_expr_list(lexer_t *lexer);
ast_expr_t *parse_expr(lexer_t *lexer);
ast_expr_t *parse_atom(lexer_t *lexer);
ast_expr_t *parse_eof(lexer_t *lexer);

bool is_binop(token_t *token) {
  return token->tag == TAG_AS
      || token->tag == TAG_PLUS
      || token->tag == TAG_MINUS
      || token->tag == TAG_TIMES
      || token->tag == TAG_DIVIDE
      || token->tag == TAG_RANGE
      || token->tag == TAG_AND
      || token->tag == TAG_OR
      || token->tag == TAG_MOD
      || token->tag == TAG_GT
      || token->tag == TAG_GE
      || token->tag == TAG_LT
      || token->tag == TAG_LE
      || token->tag == TAG_EQ
      || token->tag == TAG_NE;
}

uint8_t binop_preced(token_t *token) {
  switch (token->tag) {
    case TAG_AS:
      return PRECED_CAST;
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
    case TAG_NE:
      return PRECED_EQ;
    case TAG_AND:
      return PRECED_AND;
    case TAG_OR:
      return PRECED_OR;
    case TAG_RANGE:
      return PRECED_RANGE;
    default:
      return PRECED_NONE;
      break;
  }
}

ast_reserved_callable_type_t ast_callable_type_for_tag(tag_t tag) {
  switch (tag) {
    case TAG_PRINT: return AST_CALL_PRINT;
    case TAG_INPUT: return AST_CALL_INPUT;
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

ast_expr_list_t *empty_expr_list() {
  ast_expr_list_t *node = malloc(sizeof(ast_expr_list_t));
  node->root = ast_empty();
  return node;
}

ast_expr_list_t *parse_expr_list(lexer_t *lexer) {
  ast_expr_list_t *node = empty_expr_list();
  ast_expr_list_t *root = node;

  ast_expr_t *e = parse_expr(lexer);
  node->root = e;
  
  while (lexer->token.tag == TAG_COMMA) {
    advance(lexer);
    ast_expr_list_t *next = malloc(sizeof(ast_expr_list_t));
    e = parse_expr(lexer);
    next->root = e;

    node->next = next;
    node = next;
  }

  node->next = NULL;
  return root;
}

ast_expr_list_t *parse_block(lexer_t *lexer) {
  ast_expr_list_t *node = empty_expr_list();
  ast_expr_list_t *root = node;

  ast_expr_t *e = parse_expr(lexer);
  node->root = e;

  while (lexer->token.tag == TAG_EOL) {
    advance(lexer);
    ast_expr_list_t *next = malloc(sizeof(ast_expr_list_t));
    e = parse_expr(lexer);
    next->root = e;

    node->next = next;
    node = next;
    if (lexer->token.tag == TAG_END) goto done;
  }

done:
  node->next = NULL;
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
      case TAG_AS:     lhs = ast_cast(lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_PLUS:   lhs = ast_binop(AST_ADD,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_MINUS:  lhs = ast_binop(AST_SUB,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_TIMES:  lhs = ast_binop(AST_MUL,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_DIVIDE: lhs = ast_binop(AST_DIV,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_MOD:    lhs = ast_binop(AST_MOD,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_AND:    lhs = ast_binop(AST_AND,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_OR:     lhs = ast_binop(AST_OR,    lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_GT:     lhs = ast_binop(AST_GT,    lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_GE:     lhs = ast_binop(AST_GE,    lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_LT:     lhs = ast_binop(AST_LT,    lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_LE:     lhs = ast_binop(AST_LE,    lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_EQ:     lhs = ast_binop(AST_EQ,    lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_NE:     lhs = ast_binop(AST_NE,    lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_RANGE:  lhs = ast_range(lhs, _parse_expr(lexer, next_min_preced)); break;

      default:
        printf("what? why this %s?\n", tag_names[tag]);
        return lhs;
    }
  }

done:
  return lhs;
}

ast_expr_t *parse_expr(lexer_t *lexer) {
  switch(lexer->token.tag) {
    case TAG_BEGIN: {
      lexer->depth++;
      advance(lexer);
      if (lexer->token.tag != TAG_END) {
        ast_expr_list_t *es = parse_block(lexer);
        if (!eat(lexer, TAG_END)) {
          free(es);
          goto error;
        }
        return ast_block(es);
      }
      if (!eat(lexer, TAG_END)) goto error;
      return ast_block(empty_expr_list());
    }
    case TAG_END: {
      // This ends up getting parsed recursively by the BEGIN handler.
      // Treat it as a no-op.
      lexer->err = NO_ERROR;
      lexer->depth--;
      return ast_empty();
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
    case TAG_WHILE: {
      advance(lexer);
      ast_expr_t *cond = parse_expr(lexer);
      if (cond->type == AST_EMPTY) goto error;
      if (!eat(lexer, TAG_DO)) goto error;
      ast_expr_t *pred = parse_expr(lexer);
      if (pred->type == AST_EMPTY) goto error;
      return ast_while_loop(cond, pred);
    }
    case TAG_FOR: {
      advance(lexer);
      ast_expr_t *index = parse_expr(lexer);
      if (index->type != AST_IDENT) goto error;
      if (!eat(lexer, TAG_IN)) goto error;
      ast_expr_t *range = parse_expr(lexer);
      if (!eat(lexer, TAG_DO)) goto error;
      ast_expr_t *pred = parse_expr(lexer);
      if (pred->type == AST_EMPTY) goto error;
      return ast_for_loop(index, range, pred);
    }
    case TAG_INPUT: {
      ast_reserved_callable_type_t callable_type = ast_callable_type_for_tag(lexer->token.tag);
      advance(lexer);
      if (lexer->token.tag == TAG_LPAREN) {
        eat(lexer, TAG_LPAREN);
        // More than 0 args.
        if (lexer->token.tag != TAG_RPAREN) {
          ast_expr_list_t *es = parse_expr_list(lexer);
          if (!eat(lexer, TAG_RPAREN)) {
            free(es);
            goto error;
          }
          return ast_reserved_callable(callable_type, es);
        }
        if (!eat(lexer, TAG_RPAREN)) goto error;
        return ast_reserved_callable(callable_type, empty_expr_list());
      }
      return ast_ident(lexer->token.string);
    }
  }

  return _parse_expr(lexer, PRECED_NONE);

error:
  lexer->err_pos = (int) lexer->pos;

  // Incomplete input?
  if (lexer->depth > 1 &&
      (lexer->token.tag == TAG_EOF || lexer->token.tag == TAG_EOL)) {
    lexer->err = LEX_INCOMPLETE_INPUT;
  } else {
    printf("Expected atom or block; got %s.\n", tag_names[lexer->token.tag]);
    lexer->err = LEX_ERROR;
  }

  return ast_empty();
}

/*
 * F -> int
 * F -> float
 * F -> ident
 * F -> ( E )
 * F -> callable ( expr-list )
 */
ast_expr_t *parse_atom(lexer_t *lexer) {

  switch (lexer->token.tag) {
    case TAG_NIL: {
      advance(lexer);
      return ast_nil();
    }
    case TAG_TYPE_INT: {
      advance(lexer);
      return ast_type(AST_INT);
    }
    case TAG_TYPE_FLOAT: {
      advance(lexer);
      return ast_type(AST_FLOAT);
    }
    case TAG_TYPE_CHAR: {
      advance(lexer);
      return ast_type(AST_CHAR);
    }
    case TAG_TYPE_STRING: {
      advance(lexer);
      return ast_type(AST_STRING);
    }
    case TAG_TYPE_BOOLEAN: {
      advance(lexer);
      return ast_type(AST_BOOLEAN);
    }
    case TAG_TRUE: {
      advance(lexer);
      return ast_boolean(true);
    }
    case TAG_FALSE: {
      advance(lexer);
      return ast_boolean(false);
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
    case TAG_CHAR: {
      char c = lexer->token.ch;
      advance(lexer);
      return ast_char(c);
    }
    case TAG_STRING: {
      ast_expr_t *e = ast_string(lexer->token.string);
      advance(lexer);
      return e;
    }
    // TODO: Not actually right - doesn't do -(3+2), for example
    case TAG_MINUS: {
      advance(lexer);
      ast_expr_t *e = parse_atom(lexer);
      if (e->type == AST_INT) e->intval *= -1;
      if (e->type == AST_FLOAT) e->floatval *= -1;
      return e;
    }
    case TAG_LPAREN: {
      advance(lexer);
      ast_expr_t *e = parse_expr(lexer);
      if (!eat(lexer, TAG_RPAREN)) goto error;
      return e;
    }
    case TAG_MUT: {
      advance(lexer);
      ast_expr_t *id = ast_ident(lexer->token.string);
      if (!eat(lexer, TAG_IDENT)) goto error;
      if (!eat(lexer, TAG_ASSIGN)) goto error;
      ast_expr_t *val = parse_expr(lexer);
      return ast_assign(id, val, F_VAR);
    }
    case TAG_IDENT: {
      ast_expr_t *id = ast_ident(lexer->token.string);
      advance(lexer);
      if (lexer->token.tag == TAG_ASSIGN) {
        advance(lexer);
        return ast_assign(id, parse_expr(lexer), F_NONE);
      }
      return id;
    }
    case TAG_DEL: {
      advance(lexer);
      ast_expr_t *id = ast_ident(lexer->token.string);
      return ast_delete(id);
    }
    case TAG_ABS: 
    case TAG_SIN: 
    case TAG_COS: 
    case TAG_TAN: 
    case TAG_SQRT: 
    case TAG_EXP: 
    case TAG_LN: 
    case TAG_LOG:
    case TAG_PRINT: {
      ast_reserved_callable_type_t callable_type = ast_callable_type_for_tag(lexer->token.tag);
      advance(lexer);
      if (lexer->token.tag == TAG_LPAREN) {
        eat(lexer, TAG_LPAREN);
        // More than 0 args.
        if (lexer->token.tag != TAG_RPAREN) {
          ast_expr_list_t *es = parse_expr_list(lexer);
          if (!eat(lexer, TAG_RPAREN)) goto error;
          return ast_reserved_callable(callable_type, es);
        }
        if (!eat(lexer, TAG_RPAREN)) goto error;
        return ast_reserved_callable(callable_type, empty_expr_list());
      }
      return ast_ident(lexer->token.string);
    }
    default: goto error;
  }

error:
  lexer->err_pos = (int) lexer->pos;

  // Incomplete input?
  if (lexer->depth > 1 &&
      (lexer->token.tag == TAG_EOF || lexer->token.tag == TAG_EOL)) {
    lexer->err = LEX_INCOMPLETE_INPUT;
  } else {
    printf("Expected atom or block; got %s.\n", tag_names[lexer->token.tag]);
    lexer->err = LEX_ERROR;
  }

  return ast_empty();
}

void parse_program(char *input, ast_expr_t *ast, parse_result_t *parse_result) {
  lexer_t lexer;

  lexer_init(&lexer, input, strlen(input));

  // Lexer error. Don't parse.
  if (lexer.err != NO_ERROR) {
    parse_result->err = lexer.err;
    parse_result->pos = lexer.pos;
    return;
  }

  // This feels so wrong.
  ast_expr_t *p = parse_start(&lexer);

  parse_result->err = lexer.err;
  parse_result->pos = lexer.pos;
  parse_result->depth = lexer.depth;
  if (lexer.err == LEX_INCOMPLETE_INPUT) {
    return;
  }

  // This feels even more wrong.
  ast->type = p->type;
  ast->flags = p->flags;
  // End up assigning the one that isn't null.
  ast->intval = p->intval;
  ast->floatval = p->floatval;
  ast->stringval = p->stringval;
  ast->charval = p->charval;
}

