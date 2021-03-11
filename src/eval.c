#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "../inc/err.h"
#include "../inc/eval.h"
#include "../inc/parser.h"
#include "../inc/ast.h"
#include "../inc/env.h"

size_t MAX_INPUT_LINE = 80;
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

void int_to_string(obj_t *obj) {
  int i = obj->intval;
  // Longest thing we can print is -2147483648, which is 11 characters + 1 for the null.
  obj->stringval = malloc(12);
  snprintf(obj->stringval, 12, "%d", i);
  obj->type = TYPE_STRING;
}

void float_to_string(obj_t *obj) {
  float f = obj->floatval;
  int len = snprintf(NULL, 0, "%f", f);
  obj->stringval = malloc(len + 1);
  snprintf(obj->stringval, len + 1, "%f", f);
  obj->type = TYPE_STRING;
}

error_t string_to_int(obj_t *obj) {
  char *end;
  long l = strtol(obj->stringval, &end, 10);
  // Expect either end of line, or to have encountered a decimal point.
  if (*end != '\0' && *end != '.') return EVAL_BAD_INPUT;
  if (l > INT_MAX)  return OVERFLOW_ERROR;
  if (l < INT_MIN)  return UNDERFLOW_ERROR;

  obj->type = TYPE_INT;
  obj->intval = (int) l;
  return NO_ERROR;
}

error_t string_to_float(obj_t *obj) {
  char *end;
  float f = strtof(obj->stringval, &end);
  if (*end != '\0') return EVAL_BAD_INPUT;

  obj->type = TYPE_FLOAT;
  obj->floatval = f;
  return NO_ERROR;
}

void eval_string_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_STRING) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }
  obj_t* obj = malloc(sizeof(obj_t));
  char* stringval = malloc(strlen(expr->stringval));
  strcpy(stringval, expr->stringval);
  obj->type = TYPE_STRING;
  obj->stringval = stringval;
  result->obj = obj;
}

void cast(obj_t *obj, obj_t *type_obj, eval_result_t *result) {
  if ( !(type_obj->type == TYPE_INT
      || type_obj->type == TYPE_FLOAT
      || type_obj->type == TYPE_CHAR
      || type_obj->type == TYPE_STRING
      || type_obj->type == TYPE_BOOLEAN)) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }

  switch (obj->type) {
    case TYPE_INT:
      switch (type_obj->type) {
        case TYPE_INT:
          goto done;
        case TYPE_FLOAT:
          obj->floatval= (float) obj->intval;
          goto done;
        case TYPE_CHAR:
          if (obj->intval < 0 || obj->intval > 255) {
            result->err = VALUE_TOO_LARGE_FOR_CHAR;
            goto done;
          }
          obj->charval= (char) obj->intval;
          goto done;
        case TYPE_STRING:
          int_to_string(obj);
          goto done;
        case TYPE_BOOLEAN:
          obj->intval = obj->intval ? 1 : 0;
          goto done;
        default:
          result->err = EVAL_TYPE_ERROR;
          goto done;
      }
      break;
    case TYPE_FLOAT:
      switch (type_obj->type) {
        case TYPE_INT:
          if (obj->floatval > INT_MAX) { result->err = OVERFLOW_ERROR; goto done; }
          if (obj->floatval < INT_MIN) { result->err = OVERFLOW_ERROR; goto done; }
          obj->intval = (int) obj->floatval;
          goto done;
        case TYPE_FLOAT:
          goto done;
        case TYPE_CHAR:
          result->err = EVAL_TYPE_ERROR;
          goto done;
        case TYPE_STRING:
          float_to_string(obj);
          goto done;
        case TYPE_BOOLEAN:
          obj->intval = (obj->floatval != 0.0f) ? (int) 1 : (int) 0;
          goto done;
        default:
          result->err = EVAL_TYPE_ERROR;
          goto done;
      }
      break;
    case TYPE_STRING:
      obj->type = type_obj->type;
      switch (type_obj->type) {
        case TYPE_INT:
          result->err = string_to_int(obj);
          goto done;
        case TYPE_FLOAT:
          result->err = string_to_float(obj);
          goto done;
        case TYPE_CHAR:
          if (strlen(obj->stringval) > 1) {
            result->err = EVAL_TYPE_ERROR;
            goto done;
          }
          obj->charval = obj->stringval[0];
          goto done;
        case TYPE_STRING:
          goto done;
        case TYPE_BOOLEAN:
          obj->intval = strlen(obj->stringval) > 0 ? 1 : 0;
          goto done;
        default:
          result->err = EVAL_TYPE_ERROR;
          goto done;
      }
      break;
    case TYPE_CHAR:
      switch (type_obj->type) {
        case TYPE_INT:
          obj->intval = (int) obj->charval;
          goto done;
        case TYPE_FLOAT:
          obj->floatval = (float) obj->charval;
          goto done;
        case TYPE_CHAR:
          goto done;
        case TYPE_STRING: {
          char c = obj->charval;
          obj->stringval = malloc(2);
          snprintf(obj->stringval, 2, "%c", c);
          goto done;
        }
        case TYPE_BOOLEAN:
          obj->intval = obj->charval != 0 ? 1 : 0;
          goto done;
        default:
          result->err = EVAL_TYPE_ERROR;
          goto done;
      }
      break;
    case TYPE_BOOLEAN:
      switch (type_obj->type) {
        case TYPE_INT:
          // No change for intval.
          goto done;
        case TYPE_FLOAT:
          obj->floatval = (float) obj->intval;
          goto done;
        case TYPE_CHAR:
          obj->charval = obj->intval ? 't' : 'f';
          goto done;
        case TYPE_STRING:
          obj->stringval = obj->intval ? "true" : "false";
          goto done;
        case TYPE_BOOLEAN:
          goto done;
        default:
          result->err = EVAL_TYPE_ERROR;
          goto done;
      }
      break;
    default:
      result->err = EVAL_TYPE_ERROR;
      break;
  }

done:
  if (result->err == NO_ERROR) obj->type = type_obj->type;
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
      (b->type == TYPE_FLOAT && b->floatval == 0.0f)) {
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
      (b->type == TYPE_FLOAT && b->floatval == 0.0f)) {
    result->err = DIVISION_BY_ZERO;
    return;
  }

  if (a->type == TYPE_INT && b->type == TYPE_INT) {
    result->obj = int_obj(a->intval % b->intval);
  } else if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
    result->obj = float_obj((float) fmod(a->intval, (double) b->floatval));
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
    result->obj = float_obj((float) fmod((double) a->floatval, b->intval));
  } else if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
    result->obj = float_obj((float) fmod((double) a->floatval, (double) b->floatval));
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
      case AST_NE: result->obj = boolean_obj(a->charval != b->charval); return;
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
    case AST_NE: result->obj = boolean_obj(x != y); return;
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

int print_args(ast_expr_list_t *args, eval_result_t *result, env_t *env) {
  int printed = 0;
  ast_expr_list_t *node = (ast_expr_list_t*) args;
  if (args->e->type != AST_EMPTY) {
    while(node != NULL) {
      eval_result_t *r = eval_expr(node->e, env);

      if (r->err != NO_ERROR) {
        result->err = r->err;
        result->obj = undef_obj();
        return printed;
      }

      // TODO: probably want to put the whole string result in result->obj.
      // TODO: it needs tests, one way or the other
      printed++;
      switch (r->obj->type) {
        case TYPE_INT: printf("%d ", r->obj->intval); break;
        case TYPE_FLOAT: printf("%f ", (double) r->obj->floatval); break;
        case TYPE_CHAR: printf("%c ", r->obj->charval); break;
        case TYPE_STRING: printf("%s ", r->obj->stringval); break;
        case TYPE_BOOLEAN: printf("%s ", r->obj->intval ? "true" : "false"); break;
        case TYPE_NIL: printf("Nil "); break;
        default: printf("?? "); break;
      }
      node = node->next;
    }
  }
  result->obj = no_obj();
  return printed;
}

void println_args(ast_expr_list_t *args, eval_result_t *result, env_t *env) {
  print_args(args, result, env);
  printf("\n");
}

void readln_input(eval_result_t *result) {
  char* s = malloc(MAX_INPUT_LINE);
  if (getline(&s, &MAX_INPUT_LINE, stdin) == -1) {
    result->err = INPUT_STREAM_ERROR;
  }
  // Trim trailing newline.
  int len = (int) strlen(s);
  uint32_t end = len > 0 ? (uint32_t) (len - 1) : 0;
  while (end > 0 && s[end] == '\n') s[end--] = 0;

  result->obj = string_obj(s);
}

void resolve_callable_expr(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  if (expr->type != AST_RESERVED_CALLABLE) {
    result->err = EVAL_TYPE_ERROR;
    return;
  }

  // Function args are stored in a list in e1.
  ast_expr_list_t *args = expr->e1;

  switch (expr->intval) {
    case AST_CALL_PRINT:
      println_args(args, result, env);
      break;
    case AST_CALL_INPUT: {
      if (print_args(args, result, env) == 0) {
        printf("? ");
      }
      readln_input(result);
      break;
    }
    case AST_CALL_ABS:
      if (args->e->type == AST_INT) {
        result->obj = int_obj(args->e->intval < 0 ? -1 * args->e->intval : args->e->intval);
      } else if (args->e->type == AST_INT)  {
        result->obj = int_obj(args->e->floatval < 0 ? -1 * (int) args->e->floatval : (int) args->e->floatval);
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

void eval_while_loop(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  eval_result_t *cond;
  eval_result_t *r;
  result->obj = nil_obj();
  result->err = NO_ERROR;

  for(;;) {
    cond = eval_expr(expr->e1, env);
    if (cond->err != NO_ERROR) {
      result->err = cond->err;
      result->obj = undef_obj();
      return;
    }

    if (!truthy(cond->obj)) return;

    r = eval_expr(expr->e2, env);
    if (r->err != NO_ERROR) {
      result->err = r->err;
      result->obj = undef_obj();
      return;
    }

    result->obj = r->obj;
  }
}

void eval_for_loop(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  char* index_name = ((ast_expr_t*) expr->e1)->stringval;

  // int from.
  if (((obj_t*)expr->e2)->type != AST_INT) {
    printf("expected int ; got %d\n", ((obj_t*)expr->e2)->type);
    result->err = TYPE_INT_REQUIRED;
    goto error;
  }
  eval_result_t *start = eval_expr(expr->e2, env);
  if ((result->err = start->err) != NO_ERROR) goto error;

  // int to.
  if (((obj_t*)expr->e3)->type != AST_INT) {
    printf("expected int ; got %d\n", ((obj_t*)expr->e3)->type);
    result->err = TYPE_INT_REQUIRED;
    goto error;
  }
  eval_result_t *end = eval_expr(expr->e3, env);
  if ((result->err = end->err) != NO_ERROR) goto error;

  // Push a new scope and store the index variable.
  // We will mutate this variable with each iteration through the loop.
  push_scope(env);
  obj_t *index_obj = int_obj(start->obj->intval);
  put_env(env, index_name, index_obj);

  int start_val = start->obj->intval;
  int end_val = end->obj->intval;
  if (start_val < 0 || end_val < 0) {
    result->err = TYPE_POSITIVE_INT_REQUIRED;
    goto error;
  }
  eval_result_t *r;

  // These are both in the same mongo function because I don't want to have to call
  // functions with more than four args.
  if (start_val <= end_val) {
    for (int i = start_val; i <= end_val; ++i) {
      // Overflow?
      if (i < 0) { result->err = OVERFLOW_ERROR; goto error; }
      index_obj->intval = i;
      r = eval_expr(expr->e4, env);
      if ((result->err = r->err) != NO_ERROR) {
        pop_scope(env);
        goto error;
      }
    }
  } else {
    for (int i = start_val; i >= end_val; --i) {
      // Overflow?
      if (i < 0) { result->err = OVERFLOW_ERROR; goto error; }
      index_obj->intval = i;
      r = eval_expr(expr->e4, env);
      if ((result->err = r->err) != NO_ERROR) {
        pop_scope(env);
        goto error;
      }
    }
  }

  pop_scope(env);
  result->obj = r->obj;
  return;

error:
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
        case AST_CAST: {
            eval_result_t *r1 = eval_expr(expr->e1, env);
            if ((result->err = r1->err) != NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->e2, env);
            if ((result->err = r2->err) != NO_ERROR) goto error;
            cast(r1->obj, r2->obj, result);
            if (result->err != NO_ERROR) goto error;
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
        case AST_NE:
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
        case AST_BLOCK: {
            ast_expr_list_t *root = expr->e1;
            push_scope(env);
            while (root != NULL) {
              eval_result_t *r = eval_expr(root->e, env);
              if ((result->err = r->err) != NO_ERROR) {
                pop_scope(env);
                goto error;
              }

              // The last object in the block is its value.
              result->obj = r->obj;
              root = root->next;
            }
            pop_scope(env);
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
            error_t error = put_env(env, name, r->obj);
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
        case AST_WHILE_LOOP:
          eval_while_loop(expr, env, result);
          break;
        case AST_FOR_LOOP:
          eval_for_loop(expr, env, result);
          break;
        default:
          result->err = EVAL_UNHANDLED_OBJECT;
          break;
    }

error:
    return result;
}

eval_result_t *eval(env_t *env, char *input) {
  ast_expr_t *ast = ast_empty();
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(input, ast, parse_result);

  eval_result_t *r = malloc(sizeof(eval_result_t));
  r->err = parse_result->err;
  r->depth = parse_result->depth;

  if (parse_result->err != NO_ERROR) {
    r->obj = no_obj();
    return r;
  }

#ifdef DEBUG
    pretty_print(ast);
#endif

  return eval_expr(ast, env);
}

