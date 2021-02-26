#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "../inc/err.h"
#include "../inc/eval.h"
#include "../inc/parser.h"
#include "../inc/ast.h"
#include "../inc/env.h"

eval_result_t *eval_expr(ast_expr_t *expr, env_t *env);

void eval_nil_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_NIL) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }
  obj_t* obj = malloc(sizeof(obj_t));
  obj->type = TYPE_NIL;
  result->obj = obj;
}

void eval_boolean_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_BOOLEAN) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }
  result->obj = boolean_obj(expr->intval == 1);
}

void eval_int_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_INT) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }
  result->obj = int_obj(expr->intval);
}

void eval_float_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_FLOAT) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }
  result->obj = float_obj(expr->floatval);
}

void eval_char_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_CHAR) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }
  result->obj = char_obj(expr->charval);
}

void eval_string_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_STRING) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }
  obj_t* obj = malloc(sizeof(obj_t));
  char* stringval = malloc(sizeof(expr->stringval) + 1);
  strcpy(stringval, expr->stringval);
  obj->type = TYPE_STRING;
  obj->stringval = stringval;
  result->obj = obj;
}

bool is_numeric(obj_t *o) {
  return o->type == TYPE_INT || o->type == TYPE_FLOAT;
}

void add(obj_t *a, obj_t *b, eval_result_t *result) {
  if (!is_numeric(a) || !is_numeric(b)) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }

  if ((a->type == TYPE_INT) && (b->type == TYPE_INT)) {
    result->obj = int_obj(a->intval + b->intval);
  } else if ((a->type == TYPE_INT) && (b->type == TYPE_FLOAT)) {
    result->obj = float_obj(a->intval + b->floatval);
  } else if ((a->type == TYPE_FLOAT) && (b->type == TYPE_INT)) {
    result->obj = float_obj(a->floatval + b->intval);
  } else if ((a->type == TYPE_FLOAT) && (b->type == TYPE_FLOAT)) {
    result->obj = float_obj(a->floatval + b->floatval);
  } else {
    result->err = EVAL_TYPE_ERROR;
  }
}

void subtract(obj_t *a, obj_t *b, eval_result_t *result) {
  if (!is_numeric(a) || !is_numeric(b)) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }

  if (a->type == TYPE_INT && b->type == TYPE_INT) {
    result->obj = int_obj(a->intval - b->intval);
  } else if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
    result->obj = float_obj(a->intval - b->floatval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
    result->obj = float_obj(a->floatval - b->intval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
    result->obj = float_obj(a->floatval - b->floatval);
  } else {
    result->err = EVAL_TYPE_ERROR;
  }
}

void multiply(obj_t *a, obj_t *b, eval_result_t *result) {
  if (!is_numeric(a) || !is_numeric(b)) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }

  if (a->type == TYPE_INT && b->type == TYPE_INT) {
    result->obj = int_obj(a->intval * b->intval);
  } else if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
    result->obj = float_obj(a->intval * b->floatval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
    result->obj = float_obj(a->floatval * b->intval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
    result->obj = float_obj(a->floatval * b->floatval);
  } else {
    result->err = EVAL_TYPE_ERROR;
  }
}

void divide(obj_t *a, obj_t *b, eval_result_t *result) {
  if (!is_numeric(a) || !is_numeric(b)) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }

  if ((b->type == TYPE_INT && b->intval == 0) ||
      (b->type == TYPE_FLOAT && b->floatval == 0.0)) {
    result->err = DIVISION_BY_ZERO;
    return;
  }

  if (a->type == TYPE_INT && b->type == TYPE_INT) {
    // Return an int if possible. I guess?
    float f_div = (float) a->intval / b->intval;
    int i_div = a->intval / b->intval;
    result->obj = (f_div == i_div) ? int_obj(i_div) : float_obj(f_div);
  } else if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
    result->obj = float_obj(a->intval / b->floatval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
    result->obj = float_obj(a->floatval / b->intval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
    result->obj = float_obj(a->floatval / b->floatval);
  } else {
    result->err = EVAL_TYPE_ERROR;
  }
}

void modulus(obj_t *a, obj_t *b, eval_result_t *result) {
  if (!is_numeric(a) || !is_numeric(b)) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }

  if ((b->type == TYPE_INT && b->intval == 0) ||
      (b->type == TYPE_FLOAT && b->floatval == 0.0)) {
    result->err = DIVISION_BY_ZERO;
    return;
  }

  if (a->type == TYPE_INT && b->type == TYPE_INT) {
    result->obj = int_obj(a->intval % b->intval);
  } else if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
    result->obj = float_obj(fmod(a->intval, b->floatval));
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
    result->obj = float_obj(fmod(a->floatval, b->intval));
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
    result->obj = float_obj(fmod(a->floatval, b->floatval));
  } else {
    result->err = EVAL_TYPE_ERROR;
  }
}

void cmp(ast_type_t type, obj_t *a, obj_t *b, eval_result_t *result) {
  if (a->type == TYPE_CHAR && b->type == TYPE_CHAR) {
    switch(type) {
      case AST_GT: result->obj = boolean_obj(a->charval >  b->charval); return;
      case AST_GE: result->obj = boolean_obj(a->charval >= b->charval); return;
      case AST_LT: result->obj = boolean_obj(a->charval <  b->charval); return;
      case AST_LE: result->obj = boolean_obj(a->charval <= b->charval); return;
      case AST_EQ: result->obj = boolean_obj(a->charval == b->charval); return;
      default:
        printf("what what what???\n");
        result->obj = boolean_obj(false);
        return;
    }
  }

  if (!is_numeric(a) || !is_numeric(b)) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }

  float x = a->type == TYPE_INT ? a->intval : a->floatval;
  float y = b->type == TYPE_INT ? b->intval : b->floatval;

  switch(type) {
    case AST_GT: result->obj = boolean_obj(x >  y); return;
    case AST_GE: result->obj = boolean_obj(x >= y); return;
    case AST_LT: result->obj = boolean_obj(x <  y); return;
    case AST_LE: result->obj = boolean_obj(x <= y); return;
    case AST_EQ: result->obj = boolean_obj(x == y); return;
    default:
      printf("what what what???\n");
      result->obj = boolean_obj(false);
      return;
  }
}

void boolean_and(obj_t *a, obj_t *b, eval_result_t *result) {
  result->obj = boolean_obj(truthy(a) && truthy(b)); 
}

void boolean_or(obj_t *a, obj_t *b, eval_result_t *result) {
  result->obj = boolean_obj(truthy(a) || truthy(b)); 
}

void resolve_callable_expr(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  if (expr->type != AST_RESERVED_CALLABLE) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }

  // Function args are stored in a list in e1.
  ast_expr_list_t *args = expr->e1;

  switch (expr->intval) {
    case AST_CALL_PRINT: {
      ast_expr_list_t *node = (ast_expr_list_t*) args;
      if (node->e->type != AST_EMPTY) {
        while(node != NULL) {
          eval_result_t *r = eval_expr(node->e, env);

          if (r->err != NO_ERROR) {
            result->err = r->err;
            result->obj = undef_obj();
            return;
          }

          // TODO: probably want to put the whole string result in result->obj.
          // TODO: it needs tests, one way or the other
          switch (r->obj->type) {
            case TYPE_INT: printf("%d ", r->obj->intval); break;
            case TYPE_FLOAT: printf("%f ", r->obj->floatval); break;
            case TYPE_CHAR: printf("%c ", r->obj->charval); break;
            case TYPE_STRING: printf("%s ", r->obj->stringval); break;
            case TYPE_BOOLEAN: printf("%s ", r->obj->intval ? "true" : "false"); break;
            case TYPE_NIL: printf("Nil "); break;
            default: printf("?? "); break;
          }
          node = node->next;
        }
        printf("\n");
      }
      result->obj = no_obj();
      break;
    }
    case AST_CALL_ABS:
      if (args->e->type == AST_INT) {
        result->obj = int_obj(args->e->intval < 0 ? -1 * args->e->intval : args->e->intval);
      } else if (args->e->type == AST_INT)  {
        result->obj = int_obj(args->e->floatval < 0 ? -1 * args->e->floatval : args->e->floatval);
      } else {
        printf("What is this slish?\n");
        goto error;
      }
      break;
    default:
      goto error;
      break;
  }

  return;

error:
      result->err = AST_TYPE_UNHANDLED;
      result->obj = undef_obj();
}

eval_result_t *eval_expr(ast_expr_t *expr, env_t *env) {
    eval_result_t *result = malloc(sizeof(eval_result_t));
    result->err = NO_ERROR;

    switch(expr->type) {
        case AST_EMPTY: {
            result->obj = no_obj();
            break;
        }
        case AST_ADD: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            if ((result->err = r1->err) != NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->e2, env);
            if ((result->err = r2->err) != NO_ERROR) goto error;
            add(r1->obj, r2->obj, result);
            if (result->err != NO_ERROR) goto error;
            break;
        }
        case AST_SUB: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            if ((result->err = r1->err) != NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->e2, env);
            if ((result->err = r2->err) != NO_ERROR) goto error;
            subtract(r1->obj, r2->obj, result);
            if (result->err != NO_ERROR) goto error;
            break;
        }
        case AST_MUL: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            if ((result->err = r1->err) != NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->e2, env);
            if ((result->err = r2->err) != NO_ERROR) goto error;
            multiply(r1->obj, r2->obj, result);
            if (result->err != NO_ERROR) goto error;
            break;
        }
        case AST_DIV: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            if ((result->err = r1->err) != NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->e2, env);
            if ((result->err = r2->err) != NO_ERROR) goto error;
            divide(r1->obj, r2->obj, result);
            if (result->err != NO_ERROR) goto error;
            break;
        }
        case AST_MOD: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            if ((result->err = r1->err) != NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->e2, env);
            if ((result->err = r2->err) != NO_ERROR) goto error;
            modulus(r1->obj, r2->obj, result);
            if (result->err != NO_ERROR) goto error;
            break;
        }
        case AST_AND: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            if ((result->err = r1->err) != NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->e2, env);
            if ((result->err = r2->err) != NO_ERROR) goto error;
            boolean_and(r1->obj, r2->obj, result); 
            if (result->err != NO_ERROR) goto error;
            break;
        }
        case AST_OR: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            if ((result->err = r1->err) != NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->e2, env);
            if ((result->err = r2->err) != NO_ERROR) goto error;
            boolean_or(r1->obj, r2->obj, result); 
            if (result->err != NO_ERROR) goto error;
            break;
        }
        case AST_GT:
        case AST_GE:
        case AST_LT:
        case AST_LE:
        case AST_EQ: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            if ((result->err = r1->err) != NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->e2, env);
            if ((result->err = r2->err) != NO_ERROR) goto error;
            cmp(expr->type, r1->obj, r2->obj, result); 
            if (result->err != NO_ERROR) goto error;
            break;
        }
        case AST_NIL: {
            eval_nil_expr(expr, result);
            break;
        }
        case AST_BOOLEAN: {
            eval_boolean_expr(expr, result);
            break;
        }
        case AST_INT:
            eval_int_expr(expr, result);
            if (result->err != NO_ERROR) goto error;
            break;
        case AST_FLOAT:
            eval_float_expr(expr, result);
            if (result->err != NO_ERROR) goto error;
            break;
        case AST_CHAR:
            eval_char_expr(expr, result);
            if (result->err != NO_ERROR) goto error;
            break;
        case AST_STRING:
            eval_string_expr(expr, result);
            if (result->err != NO_ERROR) goto error;
            break;
        case AST_IDENT: {
            // Look up the identifier by name in the environment.
            const char* name = expr->stringval;
            obj_t *obj = get_env(env, name);
            if (obj->type == TYPE_UNDEF) {
              result->err = ENV_SYMBOL_UNDEFINED;
              goto error;
            }
            result->obj = obj;
            break;
        }
        case AST_RESERVED_CALLABLE: {
          resolve_callable_expr(expr, env, result);
          if (result->err != NO_ERROR) goto error;
          break;
        }
        case AST_ASSIGN: {
            // Save the expression associated with the identifier.
            const char* name = ((ast_expr_t*)expr->e1)->stringval;
            // Eval the object now and save the result as a primitive value.
            eval_result_t *r = eval_expr(expr->e2, env);
            if ((result->err = r->err) != NO_ERROR) goto error;
            int error = put_env(env, name, r->obj);
            result->obj = r->obj;
            // Store the obj in the result value for the caller.
            if (error != NO_ERROR) {
              result->err = error;
              goto error;
            }
            break;
        }
        case AST_IF_THEN: {
          eval_result_t *if_val = eval_expr(expr->e1, env);
          result->err = if_val->err;
          if (result->err != NO_ERROR) goto error;
          if (truthy(if_val->obj)) {
            eval_result_t *then_val = eval_expr(expr->e2, env);
            result->err = then_val->err;
            if (result->err != NO_ERROR) goto error;
            result->obj = then_val->obj;
            break;
          }
          result->obj = nil_obj();
          break;
        }
        case AST_IF_THEN_ELSE: {
          eval_result_t *if_val = eval_expr(expr->e1, env);
          if ((result->err = if_val->err) != NO_ERROR) goto error;
          if (truthy(if_val->obj)) {
            eval_result_t *then_val = eval_expr(expr->e2, env);
            if ((result->err = then_val->err) != NO_ERROR) goto error;
            result->obj = then_val->obj;
            break;
          } else {
            eval_result_t *else_val = eval_expr(expr->e3, env);
            if ((result->err = else_val->err) != NO_ERROR) goto error;
            result->obj = else_val->obj;
            break;
          }
          result->obj = nil_obj();
          break;
        }
        case AST_FOR_LOOP: {
          char* index_name = ((ast_expr_t*) expr->e1)->stringval;

          eval_result_t *start = eval_expr(expr->e2, env);
          if ((result->err = start->err) != NO_ERROR) goto error;

          eval_result_t *end = eval_expr(expr->e3, env);
          if ((result->err = end->err) != NO_ERROR) goto error;

          // Hack hack
          push_scope(env);
          obj_t *index_obj = int_obj(start->obj->intval);
          put_env(env, index_name, index_obj);
          eval_result_t *r;
          for (int i = start->obj->intval; i <= end->obj->intval; ++i) {
            index_obj->intval = i;

            r = eval_expr(expr->e4, env);
            if ((result->err = r->err) != NO_ERROR) {
              pop_scope(env);
              goto error;
            }
          }
          pop_scope(env);
          result->obj = r->obj;
          break;
        }
        default:
            result->err = EVAL_UNHANDLED_OBJECT;
            break;
    }

error:
    return result;
}

eval_result_t *eval(env_t *env, char *input) {
  ast_expr_t *ast = parse_program(input);

#ifdef DEBUG
    pretty_print(ast);
#endif

  return eval_expr(ast, env);
}

