#include <stdio.h>
#include "../inc/def.h"
#include "../inc/mem.h"
#include "../inc/arr.h"
#include "../inc/str.h"
#include "../inc/ast.h"
#include "../inc/lexer.h"
#include "../inc/parser.h"

static ast_expr_t *parse_start(lexer_t *lexer);
static ast_expr_list_t *parse_expr_list(lexer_t *lexer);
static ast_expr_t *parse_expr(lexer_t *lexer);
static ast_expr_t *parse_atom(lexer_t *lexer);
static ast_expr_t *_parse_expr(lexer_t *lexer, int min_preced);
static ast_expr_t *_parse_subscript(lexer_t *lexer, int min_preced);

static boolean is_op(token_t *token) {
  return token->tag == TAG_AS
      || token->tag == TAG_PLUS
      || token->tag == TAG_MINUS
      || token->tag == TAG_TIMES
      || token->tag == TAG_DIVIDE
      || token->tag == TAG_RANGE
      || token->tag == TAG_AND
      || token->tag == TAG_OR
      || token->tag == TAG_MOD
      || token->tag == TAG_NOT
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
      || token->tag == TAG_IS
      || token->tag == TAG_IN
      || token->tag == TAG_MEMBER_ACCESS
      || token->tag == TAG_LBRACKET  // Always subscript
      || token->tag == TAG_LPAREN
      || token->tag == TAG_COLON  // Always dict kv association
      || token->tag == TAG_ASSIGN
      ;
}

static uint8_t op_preced(token_t *token) {
  switch (token->tag) {
    case TAG_MEMBER_ACCESS:
      return PRECED_MEMBER_ACCESS;
    case TAG_LPAREN:
      return PRECED_FUNCTION_CALL;
    case TAG_LBRACKET:
      return PRECED_SUBSCRIPT;
    case TAG_AS:
      return PRECED_CAST;
    case TAG_TIMES:
    case TAG_DIVIDE:
    case TAG_MOD:
      return PRECED_MUL;
    case TAG_PLUS:
    case TAG_MINUS:
      return PRECED_ADD;
    case TAG_NOT:
      return PRECED_NOT;
    case TAG_GT:
    case TAG_GE:
    case TAG_LT:
    case TAG_LE:
      return PRECED_GLT;
    case TAG_EQ:
    case TAG_NE:
    case TAG_IS:
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
    case TAG_COLON:
      return PRECED_MAPS_TO;
    case TAG_ASSIGN:
      return PRECED_ASSIGN;
    default:
      return PRECED_NONE;
      break;
  }
}

static int op_preced_inc(token_t *token) {
  // Return -1 for right-associative ops
  return 1;
}

static ast_reserved_callable_type_t ast_callable_type_for_tag(tag_t tag) {
  switch (tag) {
    case TAG_TYPEOF: return AST_CALL_TYPE_OF;
    case TAG_TO_HEX: return AST_CALL_TO_HEX;
    case TAG_TO_BIN: return AST_CALL_TO_BIN;
    case TAG_DUMP: return AST_CALL_DUMP;
    case TAG_PRINT: return AST_CALL_PRINT;
    case TAG_INPUT: return AST_CALL_INPUT;
    case TAG_RAND: return AST_CALL_RAND;
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

static ast_expr_t *parse_start(lexer_t *lexer) {
  ast_expr_t *e = ast_empty();
  while(lexer->token.tag != TAG_EOF) {
    e = parse_expr(lexer);
    advance(lexer);
  }
  return e;
}

static ast_expr_list_t *empty_expr_list(void) {
  ast_expr_list_t *node = mem_alloc(sizeof(ast_expr_list_t));
  node->root = ast_empty();
  node->next = NULL;
  return node;
}

static ast_expr_kv_list_t *empty_expr_kv_list(void) {
  ast_expr_kv_list_t *node = mem_alloc(sizeof(ast_expr_kv_list_t));
  node->k = ast_empty();
  node->v = ast_empty();
  node->next = NULL;
  return node;
}

static ast_fn_arg_decl_t *parse_fn_arg_decl(lexer_t *lexer) {
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

static ast_expr_list_t *parse_expr_list(lexer_t *lexer) {
  ast_expr_list_t *node = empty_expr_list();
  ast_expr_list_t *root = node;

  ast_expr_t *e = parse_expr(lexer);
  node->root = e;
  
  while (lexer->token.tag == TAG_COMMA) {
    advance(lexer);
    ast_expr_list_t *next = mem_alloc(sizeof(ast_expr_list_t));
    e = parse_expr(lexer);
    next->root = e;
    next->next = NULL;

    node->next = next;
    node = next;
  }

  return root;
}

static ast_expr_list_t *parse_list_expr_list(lexer_t *lexer) {
  ast_expr_list_t *node = empty_expr_list();
  ast_expr_list_t *root = node;
  node->root = NULL;

  ast_expr_t *e = parse_expr(lexer);
  if (e->type > AST_EMPTY) node->root = e;

  while (lexer->token.tag == TAG_COMMA || lexer->token.tag == TAG_EOL) {
    advance(lexer);
    ast_expr_list_t *next = mem_alloc(sizeof(ast_expr_list_t));
    e = parse_expr(lexer);
    if (e->type > AST_EMPTY) {

      // This is ghastly.
      if (node->root == NULL) {
        node->root = e;
      } else {
        next->root = e;
        next->next = NULL;

        node->next = next;
        node = next;
      }
    }
  }

  return root;
}

static ast_expr_kv_list_t *parse_expr_kv_list(lexer_t *lexer) {
  ast_expr_kv_list_t *node = empty_expr_kv_list();
  ast_expr_kv_list_t *root = node;

  ast_expr_t *e = parse_expr(lexer);
  if (e->type != AST_MAPS_TO) {
    printf("Syntax error.");
    return empty_expr_kv_list();
  }

  root->k = e->op_args->a;
  root->v = e->op_args->b;
  root->next = NULL;

  while (lexer->token.tag == TAG_COMMA) {
    advance(lexer);
    ast_expr_kv_list_t *next = mem_alloc(sizeof(ast_expr_kv_list_t));
    e = parse_expr(lexer);

    if (e->type != AST_MAPS_TO) {
      printf("Syntax error. got %s\n.", ast_node_names[e->type]);
      return empty_expr_kv_list();
    }

    next->k = e->op_args->a;
    next->v = e->op_args->b;
    next->next = NULL;

    node->next = next;
    node = next;
  }

  return root;
}

static ast_expr_list_t *parse_block(lexer_t *lexer) {
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

ast_expr_t *_parse_subscript(lexer_t *lexer, int min_preced) {
  ast_expr_t *expr = parse_expr(lexer);

  if (!eat(lexer, TAG_RBRACKET)) {
    printf("Expected to eat right bracket. Could not eat tasty right bracket.\n");
    lexer->err = ERR_SYNTAX_ERROR;
    return ast_empty();
  }

  return expr;
}

static ast_expr_list_t *_parse_fn_args(lexer_t *lexer, int min_preced) {
  // Emtpy arglist?
  if (lexer->token.tag == TAG_RPAREN) {
    eat(lexer, TAG_RPAREN);
    return empty_expr_list();
  }

  ast_expr_list_t *args = parse_expr_list(lexer);
  if (!eat(lexer, TAG_RPAREN)) {
    printf("Expected to eat right paren. Could not eat tasty right paren.\n");
    lexer->err = ERR_SYNTAX_ERROR;
    return empty_expr_list();
  }

  return args;
}

static ast_expr_t *_parse_expr(lexer_t *lexer, int min_preced) {
  ast_expr_t *lhs = parse_atom(lexer);

  // The recursively recursive precedence-climbing algorithm.
  while (1) {
    token_t token = lexer->token;

    if (!is_op(&token) ||
        op_preced(&token) < min_preced) {
      goto done;
    }

    int preced = op_preced(&token);
    int preced_inc = op_preced_inc(&token);

    // TODO the +1 is only for left-associative operators.
    int next_min_preced = preced + preced_inc;

    tag_t tag = token.tag;
    advance(lexer);

    switch(tag) {
      case TAG_ASSIGN:         lhs = ast_op(AST_ASSIGN,        lhs, parse_expr(lexer)); break;
      case TAG_PLUS:           lhs = ast_op(AST_ADD,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_MINUS:          lhs = ast_op(AST_SUB,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_TIMES:          lhs = ast_op(AST_MUL,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_DIVIDE:         lhs = ast_op(AST_DIV,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_MOD:            lhs = ast_op(AST_MOD,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_BITWISE_OR:     lhs = ast_op(AST_BITWISE_OR,    lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_BITWISE_XOR:    lhs = ast_op(AST_BITWISE_XOR,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_BITWISE_AND:    lhs = ast_op(AST_BITWISE_AND,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_BITWISE_SHL:    lhs = ast_op(AST_BITWISE_SHL,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_BITWISE_SHR:    lhs = ast_op(AST_BITWISE_SHR,   lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_AND:            lhs = ast_op(AST_AND,           lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_OR:             lhs = ast_op(AST_OR,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_GT:             lhs = ast_op(AST_GT,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_GE:             lhs = ast_op(AST_GE,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_LT:             lhs = ast_op(AST_LT,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_LE:             lhs = ast_op(AST_LE,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_EQ:             lhs = ast_op(AST_EQ,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_NE:             lhs = ast_op(AST_NE,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_IS:             lhs = ast_op(AST_IS,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_IN:             lhs = ast_op(AST_IN,            lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_LPAREN:         lhs = ast_func_call(            lhs, _parse_fn_args(lexer, next_min_preced)); break;
      case TAG_LBRACKET:       lhs = ast_op(AST_SUBSCRIPT,     lhs, _parse_subscript(lexer, next_min_preced)); break;
      case TAG_COLON:          lhs = ast_op(AST_MAPS_TO,       lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_AS:             lhs = ast_cast(                 lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_RANGE:          lhs = ast_range(                lhs, _parse_expr(lexer, next_min_preced)); break;
      case TAG_MEMBER_ACCESS:  lhs = ast_access(               lhs, _parse_expr(lexer, next_min_preced)); break;

      default:
        printf("what? why this %s?\n", tag_names[tag]);
        return lhs;
    }
  }

done:
  return lhs;
}

static ast_expr_t *parse_expr(lexer_t *lexer) {
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
    case TAG_DO: {
      advance(lexer);
      ast_expr_t *pred = parse_expr(lexer);
      if (pred->type == AST_EMPTY) goto error;
      if (!eat(lexer, TAG_WHILE)) goto error;
      ast_expr_t *cond = parse_expr(lexer);
      if (cond->type == AST_EMPTY) goto error;
      return ast_do_while_loop(pred, cond);
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
      // Be sure not to interpret the 'in' as part of a binop.
      ast_expr_t *index = parse_atom(lexer);
      if (index->type != AST_IDENT) goto error;
      if (!eat(lexer, TAG_IN)) goto error;
      ast_expr_t *range = parse_expr(lexer);
      ast_expr_t *pred = parse_expr(lexer);
      if (pred->type == AST_EMPTY) goto error;
      return ast_for_loop(index, range, pred);
    }
    case TAG_LIST: {
      // TODO - consolidate block parsing
      advance(lexer);
      if (lexer->token.tag == TAG_BEGIN) {
        lexer->depth++;
        eat(lexer, TAG_BEGIN);
        ast_expr_list_t *es = parse_list_expr_list(lexer);
        if (!eat(lexer, TAG_END)) {
          mem_free(es);
          goto error;
        }
        return ast_list(es);
      }
      return ast_list(NULL);
    }
    case TAG_DICT: {
      advance(lexer);
      if (lexer->token.tag == TAG_BEGIN) {
        eat(lexer, TAG_BEGIN);
        ast_expr_kv_list_t *kv = parse_expr_kv_list(lexer);
        if (!eat(lexer, TAG_END)) {
          mem_free(kv);
          goto error;
        }
        return ast_dict(kv);
      }
      return ast_dict(NULL);
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
    printf("Expected expr or block; got %s at pos %d.\n", tag_names[lexer->token.tag], lexer->pos);
    lexer->err = ERR_LEX_ERROR;
  }

  return ast_empty();
}

static ast_expr_t *parse_atom(lexer_t *lexer) {
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
    case TAG_NOT: {
      advance(lexer);
      return ast_unary(AST_NOT, parse_atom(lexer));
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
    case TAG_INVARIABLE: {
      advance(lexer);
      if (!eat(lexer, TAG_IDENT)) goto error;
      return ast_ident_decl(c_str_to_bytearray(lexer->token.string), F_NONE);
    }
    case TAG_VARIABLE: {
      advance(lexer);
      if (!eat(lexer, TAG_IDENT)) goto error;
      return ast_ident_decl(c_str_to_bytearray(lexer->token.string), F_VAR);
    }
    case TAG_IDENT: {
      ast_expr_t *id = ast_ident(c_str_to_bytearray(lexer->token.string));
      if (!eat(lexer, TAG_IDENT)) goto error;
      return id;
    }
    case TAG_ARR_DECL: {
      advance(lexer);
      if (!eat(lexer, TAG_LPAREN)) goto error;
      ast_expr_t *size = parse_expr(lexer);
      if (!eat(lexer, TAG_RPAREN)) goto error;
      return ast_array_decl(size);
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
    case TAG_IS:
    case TAG_TYPEOF:
    case TAG_RAND:
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
    case TAG_INPUT:
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
    printf("Expected atom; got %s at pos %d.\n", tag_names[lexer->token.tag], lexer->pos);
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

  ast_expr_t *p = parse_start(&lexer);

  parse_result->err = lexer.err;
  parse_result->pos = lexer.pos;
  parse_result->depth = lexer.depth;
  if (lexer.err == ERR_LEX_INCOMPLETE_INPUT) {
    mem_free(p);
    return;
  }

  mem_cp(ast, p, sizeof(ast_expr_t));
  mem_free(p);
}

