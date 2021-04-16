#include <stdio.h>
#include "../inc/def.h"
#include "../inc/mem.h"
#include "../inc/arr.h"
#include "../inc/str.h"
#include "../inc/ast.h"
#include "../inc/lexer.h"
#include "../inc/parser.h"

ast_expr_t *parse_start(lexer_t *lexer);
ast_expr_list_t *parse_expr_list(lexer_t *lexer);
ast_expr_t *parse_expr(lexer_t *lexer);
ast_expr_t *parse_atom(lexer_t *lexer);
ast_expr_t *parse_eof(lexer_t *lexer);

boolean is_binop(token_t *token) {
  return token->tag == TAG_AS
      || token->tag == TAG_PLUS
      || token->tag == TAG_MINUS
      || token->tag == TAG_TIMES
      || token->tag == TAG_DIVIDE
      || token->tag == TAG_RANGE
      || token->tag == TAG_AND
      || token->tag == TAG_OR
      || token->tag == TAG_MOD
      || token->tag == TAG_BITWISE_SHL
      || token->tag == TAG_BITWISE_SHR
      || token->tag == TAG_BITWISE_OR
      || token->tag == TAG_BITWISE_XOR
      || token->tag == TAG_BITWISE_AND
      || token->tag == TAG_GT
      || token->tag == TAG_GE
      || token->tag == TAG_LT
      || token->tag == TAG_LE
      || token->tag == TAG_EQ
      || token->tag == TAG_NE
      || token->tag == TAG_IN
      || token->tag == TAG_MEMBER_ACCESS
      ;
}

uint8_t binop_preced(token_t *token) {
  switch (token->tag) {
    case TAG_MEMBER_ACCESS:
      return PRECED_MEMBER_ACCESS;
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
    case TAG_IN:
      return PRECED_MEMBERSHIP;
    case TAG_BITWISE_AND:
      return PRECED_BITWISE_AND;
    case TAG_BITWISE_XOR:
      return PRECED_BITWISE_XOR;
    case TAG_BITWISE_OR:
      return PRECED_BITWISE_OR;
    case TAG_BITWISE_NOT:
      return PRECED_BITWISE_NOT;
    case TAG_BITWISE_SHL:
    case TAG_BITWISE_SHR:
      return PRECED_BITWISE_SHIFT;
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
    case TAG_TO_HEX: return AST_CALL_TO_HEX;
    case TAG_TO_BIN: return AST_CALL_TO_BIN;
    case TAG_DUMP: return AST_CALL_DUMP;
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
  ast_expr_t *e = ast_empty();
  while(lexer->token.tag != TAG_EOF) {
    e = parse_expr(lexer);
    advance(lexer);
  }
  return e;
}

ast_expr_list_t *empty_expr_list() {
  ast_expr_list_t *node = mem_alloc(sizeof(ast_expr_list_t));
  node->root = ast_empty();
  return node;
}

ast_fn_arg_decl_t *parse_fn_arg_decl(lexer_t *lexer) {
  ast_fn_arg_decl_t *node = mem_alloc(sizeof(ast_fn_arg_decl_t));
  ast_fn_arg_decl_t *root = node;
  node->name = NULL;
  node->next = NULL;

  if (lexer->token.tag != TAG_IDENT) return NULL;

  node->name = mem_alloc(sizeof(bytearray_t));
  node->name = c_str_to_bytearray(lexer->token.string);
  advance(lexer);

  while (lexer->token.tag == TAG_COMMA) {
    advance(lexer);

    node->next = mem_alloc(sizeof(ast_fn_arg_decl_t));
    node = node->next;

    node->name = mem_alloc(sizeof(bytearray_t));
    node->name = c_str_to_bytearray(lexer->token.string);
    advance(lexer);
  }

  node->next = NULL;
  return root;
}

ast_expr_list_t *parse_expr_list(lexer_t *lexer) {
  ast_expr_list_t *node = empty_expr_list();
  ast_expr_list_t *root = node;

  ast_expr_t *e = parse_expr(lexer);
  node->root = e;
  
  while (lexer->token.tag == TAG_COMMA) {
    advance(lexer);
    ast_expr_list_t *next = mem_alloc(sizeof(ast_expr_list_t));
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
    ast_expr_list_t *next = mem_alloc(sizeof(ast_expr_list_t));
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
      case TAG_PLUS:           lhs = ast_binop(AST_ADD,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_MINUS:          lhs = ast_binop(AST_SUB,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_TIMES:          lhs = ast_binop(AST_MUL,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_DIVIDE:         lhs = ast_binop(AST_DIV,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_MOD:            lhs = ast_binop(AST_MOD,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_BITWISE_OR:     lhs = ast_binop(AST_BITWISE_OR,    lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_BITWISE_XOR:    lhs = ast_binop(AST_BITWISE_XOR,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_BITWISE_AND:    lhs = ast_binop(AST_BITWISE_AND,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_BITWISE_SHL:    lhs = ast_binop(AST_BITWISE_SHL,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_BITWISE_SHR:    lhs = ast_binop(AST_BITWISE_SHR,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_AND:            lhs = ast_binop(AST_AND,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_OR:             lhs = ast_binop(AST_OR,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_GT:             lhs = ast_binop(AST_GT,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_GE:             lhs = ast_binop(AST_GE,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_LT:             lhs = ast_binop(AST_LT,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_LE:             lhs = ast_binop(AST_LE,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_EQ:             lhs = ast_binop(AST_EQ,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_NE:             lhs = ast_binop(AST_NE,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_IN:             lhs = ast_binop(AST_IN,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_AS:             lhs = ast_cast(                    lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_RANGE:          lhs = ast_range(                   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_MEMBER_ACCESS:  lhs = ast_access(                  lhs, _parse_expr(lexer, next_min_preced)); break;

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
          mem_free(es);
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
      lexer->err = ERR_NO_ERROR;
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
      ast_expr_t *pred = parse_expr(lexer);
      if (pred->type == AST_EMPTY) goto error;
      return ast_for_loop(index, range, pred);
    }
    case TAG_ARR_DECL: {
      advance(lexer);
      if (!eat(lexer, TAG_LPAREN)) goto error;
      ast_expr_t *size = parse_expr(lexer);
      if (!eat(lexer, TAG_RPAREN)) goto error;
      return ast_array_decl(size);
    }
    case TAG_LIST: {
      advance(lexer);
      if (!eat(lexer, TAG_OF)) goto error;
      if (lexer->token.tag != TAG_TYPE_NAME) goto error;
      ast_expr_t *type_name = parse_expr(lexer);
      if (lexer->token.tag == TAG_BEGIN) {
        eat(lexer, TAG_BEGIN);
        ast_expr_list_t *es = parse_expr_list(lexer);
        if (!eat(lexer, TAG_END)) {
          mem_free(es);
          goto error;
        }
        return ast_list(type_name->bytearray, es);
      }
      return ast_list(type_name->bytearray, NULL);
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
            mem_free(es);
            goto error;
          }
          return ast_reserved_callable(callable_type, es);
        }
        if (!eat(lexer, TAG_RPAREN)) goto error;
        return ast_reserved_callable(callable_type, empty_expr_list());
      }
      return ast_ident(c_str_to_bytearray(lexer->token.string));
    }
  }

  return _parse_expr(lexer, PRECED_NONE);

error:
  lexer->err_pos = (int) lexer->pos;

  // Incomplete input?
  if (lexer->depth > 1 &&
      (lexer->token.tag == TAG_EOF || lexer->token.tag == TAG_EOL)) {
    lexer->err = ERR_LEX_INCOMPLETE_INPUT;
  } else {
    printf("Expected atom or block; got %s.\n", tag_names[lexer->token.tag]);
    lexer->err = ERR_LEX_ERROR;
  }

  return ast_empty();
}

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
    case TAG_TYPE_BYTE: {
      advance(lexer);
      return ast_type(AST_BYTE);
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
      return ast_boolean(True);
    }
    case TAG_FALSE: {
      advance(lexer);
      return ast_boolean(False);
    }
    case TAG_HEX: {
      ast_expr_t *e = ast_int(hex_to_int(lexer->token.string));
      advance(lexer);
      return e;
    }
    case TAG_BIN: {
      ast_expr_t *e = ast_int(bin_to_int(lexer->token.string));
      advance(lexer);
      return e;
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
    case TAG_BYTE: {
      char c = lexer->token.ch;
      advance(lexer);
      return ast_byte(c);
    }
    case TAG_STRING: {
      ast_expr_t *e = ast_string(c_str_to_bytearray(lexer->token.string));
      advance(lexer);
      return e;
    }
    case TAG_MINUS: {
      advance(lexer);
      return ast_unary(AST_NEGATE, parse_atom(lexer));
    }
    case TAG_BITWISE_NOT: {
      advance(lexer);
      return ast_unary(AST_BITWISE_NOT, parse_atom(lexer));
    }
    case TAG_LPAREN: {
      advance(lexer);
      ast_expr_t *e = parse_expr(lexer);
      if (!eat(lexer, TAG_RPAREN)) goto error;
      return e;
    }
    case TAG_LBRACKET: {
      advance(lexer);
      ast_expr_t *e = parse_expr(lexer);
      if (!eat(lexer, TAG_RBRACKET)) goto error;
      return e;
    }
    case TAG_INVARIABLE: {
      advance(lexer);
      ast_expr_t *id = ast_ident(c_str_to_bytearray(lexer->token.string));
      if (!eat(lexer, TAG_IDENT)) goto error;
      if (!eat(lexer, TAG_ASSIGN)) goto error;
      ast_expr_t *val = parse_expr(lexer);
      return ast_assign_val(id, val);
    }
    case TAG_VARIABLE: {
      advance(lexer);
      ast_expr_t *id = ast_ident(c_str_to_bytearray(lexer->token.string));
      if (!eat(lexer, TAG_IDENT)) goto error;
      if (!eat(lexer, TAG_ASSIGN)) goto error;
      ast_expr_t *val = parse_expr(lexer);
      return ast_assign_var(id, val);
    }
    case TAG_IDENT: {
      ast_expr_t *id = ast_ident(c_str_to_bytearray(lexer->token.string));
      advance(lexer);
      // Identifier in sequence access?
      if (lexer->token.tag == TAG_LBRACKET) {
        ast_expr_t *index = parse_expr(lexer);
        if (lexer->token.tag == TAG_ASSIGN) {
          advance(lexer);
          // Sequence elem assignment.
          return ast_assign_elem(id, index, parse_expr(lexer));
        }
        // Sequence elem access.
        return ast_seq_elem(id, index);
      }
      // Identifier in assignment?
      if (lexer->token.tag == TAG_ASSIGN) {
        advance(lexer);
        return ast_reassign(id, parse_expr(lexer));
      }
      return id;
    }
    case TAG_FUNC_DEF: {
      advance(lexer);
      if (!eat(lexer, TAG_LPAREN)) goto error;
      ast_fn_arg_decl_t *args = NULL;
      if (lexer->token.tag != TAG_RPAREN) {
        args = parse_fn_arg_decl(lexer);
      }
      if (!eat(lexer, TAG_RPAREN)) { mem_free(args); goto error; }
      if (lexer->token.tag != TAG_BEGIN) { mem_free(args); goto error; }
      ast_expr_t *block = parse_expr(lexer);
      return ast_func_def(args, block->block_exprs);
    }
    case TAG_FUNC_CALL: {
      bytearray_t *name = c_str_to_bytearray(lexer->token.string);
      advance(lexer);
      if (!eat(lexer, TAG_LPAREN)) { mem_free(name); goto error; }
      ast_expr_list_t *args = NULL;
      if (lexer->token.tag != TAG_RPAREN) {
        args = parse_expr_list(lexer);
      }
      if (!eat(lexer, TAG_RPAREN)) { mem_free(name); mem_free(args); goto error; }
      return ast_func_call(name, args);
    }
    case TAG_FUNC_RETURN: {
      // syntax: return a, b
      // TODO: destructuring assignment
      advance(lexer);
      ast_expr_list_t *args = parse_expr_list(lexer);
      return ast_func_return(args);
    }
    case TAG_METHOD_CALL: {
      bytearray_t *name = c_str_to_bytearray(lexer->token.string);
      advance(lexer);
      if (!eat(lexer, TAG_LPAREN)) { mem_free(name); goto error; }
      if (lexer->token.tag == TAG_RPAREN) {
        ast_expr_t *id = ast_method_call(name, NULL);
        mem_free(name);
        advance(lexer);
        return id;
      }
      // More than 0 args.
      ast_expr_list_t *args = parse_expr_list(lexer);
      if (!eat(lexer, TAG_RPAREN)) { mem_free(name); goto error; }
      ast_expr_t *id = ast_method_call(name, args);
      mem_free(name);
      return id;
    }
    case TAG_TYPE_NAME: {
      ast_expr_t *type_name = ast_type_name(c_str_to_bytearray(lexer->token.string));
      advance(lexer);
      return type_name;
    }
    case TAG_DEL: {
      advance(lexer);
      if (!eat(lexer, TAG_LPAREN)) goto error;
      ast_expr_t *id = ast_ident(c_str_to_bytearray(lexer->token.string));
      if (!eat(lexer, TAG_IDENT)) { mem_free(id); goto error; }
      if (!eat(lexer, TAG_RPAREN)) { mem_free(id); goto error; }
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
    case TAG_TO_HEX:
    case TAG_TO_BIN:
    case TAG_DUMP:
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
      return ast_ident(c_str_to_bytearray(lexer->token.string));
    }
    default: goto error;
  }

error:
  lexer->err_pos = (int) lexer->pos;

  // Incomplete input?
  if (lexer->depth > 1 &&
      (lexer->token.tag == TAG_EOF || lexer->token.tag == TAG_EOL)) {
    lexer->err = ERR_LEX_INCOMPLETE_INPUT;
  } else {
    printf("Expected atom or block; got %s.\n", tag_names[lexer->token.tag]);
    lexer->err = ERR_LEX_ERROR;
  }

  return ast_empty();
}

void parse_program(char *input, ast_expr_t *ast, parse_result_t *parse_result) {
  lexer_t lexer;

  lexer_init(&lexer, input, c_str_len(input));

  // Lexer error. Don't parse.
  if (lexer.err != ERR_NO_ERROR) {
    parse_result->err = lexer.err;
    parse_result->pos = lexer.pos;
    return;
  }

  // This feels so wrong.
  ast_expr_t *p = parse_start(&lexer);

  parse_result->err = lexer.err;
  parse_result->pos = lexer.pos;
  parse_result->depth = lexer.depth;
  if (lexer.err == ERR_LEX_INCOMPLETE_INPUT) {
    return;
  }

  // This feels even more wrong.
  ast->type = p->type;
  ast->flags = p->flags;
  // End up assigning the one that isn't null.
  ast->boolval = p->boolval;
  ast->intval = p->intval;
  ast->floatval = p->floatval;
  ast->byteval = p->byteval;
  ast->bytearray = p->bytearray;
}

