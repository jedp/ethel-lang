#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../inc/err.h"
#include "../inc/eval.h"
#include "../inc/parser.h"
#include "../inc/ast.h"
#include "../inc/env.h"

void eval_int_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_INT) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }
  obj_t* obj = malloc(sizeof(obj_t));
  obj->type = TYPE_INT;
  obj->intval = expr->intval;
  result->obj = obj;
}

void eval_float_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_FLOAT) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }
  obj_t* obj = malloc(sizeof(obj_t));
  obj->type = TYPE_FLOAT;
  obj->floatval = expr->floatval;
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
    result-> obj = float_obj(a->intval + b->floatval);
  } else if ((a->type == TYPE_FLOAT) && (b->type == TYPE_INT)) {
    result-> obj = float_obj(a->floatval + b->intval);
  } else if ((a->type == TYPE_FLOAT) && (b->type == TYPE_FLOAT)) {
    result-> obj = float_obj(a->floatval + b->floatval);
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
    result-> obj = int_obj(a->intval - b->intval);
  } else if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
    result-> obj = float_obj(a->intval - b->floatval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
    result-> obj = float_obj(a->floatval - b->intval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
    result-> obj = float_obj(a->floatval - b->floatval);
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
    result-> obj = int_obj(a->intval * b->intval);
  } else if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
    result-> obj = float_obj(a->intval * b->floatval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
    result-> obj = float_obj(a->floatval * b->intval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
    result-> obj = float_obj(a->floatval * b->floatval);
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
    result-> obj = (f_div == i_div) ? int_obj(i_div) : float_obj(f_div);
  } else if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
    result-> obj = float_obj(a->intval / b->floatval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
    result-> obj = float_obj(a->floatval / b->intval);
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
    result-> obj = float_obj(a->floatval / b->floatval);
  } else {
    result->err = EVAL_TYPE_ERROR;
  }
}

eval_result_t *eval_expr(ast_expr_t *expr, env_t *env) {
    eval_result_t *result = malloc(sizeof(eval_result_t));
    result->err = NO_ERROR;

    switch(expr->type) {
        case AST_ADD: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            eval_result_t *r2 = eval_expr(expr->e2, env);
            add(r1->obj, r2->obj, result);
            if (result->err != NO_ERROR) goto error;
            break;
        }
        case AST_SUB: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            eval_result_t *r2 = eval_expr(expr->e2, env);
            subtract(r1->obj, r2->obj, result);
            if (result->err != NO_ERROR) goto error;
            break;
        }
        case AST_MUL: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            eval_result_t *r2 = eval_expr(expr->e2, env);
            multiply(r1->obj, r2->obj, result);
            if (result->err != NO_ERROR) goto error;
            break;
        }
        case AST_DIV: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            eval_result_t *r2 = eval_expr(expr->e2, env);
            divide(r1->obj, r2->obj, result);
            if (result->err != NO_ERROR) goto error;
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
        case AST_IDENT: {
            // Look up the identifier by name in the environment.
            const char* name = expr->stringval;
            obj_t *obj = get_env(env, name);
            if (obj->type == TYPE_NIL) {
              result->err = ENV_SYMBOL_UNDEFINED;
              goto error;
            }
            result->obj = obj;
            break;
        }
        case AST_ASSIGN: {
            // Save the expression associated with the identifier.
            const char* name = ((ast_expr_t*)expr->e1)->stringval;
            // Eval the object now and save the result as a primitive value.
            eval_result_t *r = eval_expr(expr->e2, env);
            int error = put_env(env, name, r->obj);
            result->obj = r->obj;
            // Store the obj in the result value for the caller.
            if (error != NO_ERROR) {
              result->err = error;
              goto error;
            }
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

