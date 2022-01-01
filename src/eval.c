#include <assert.h>
#include <stdio.h>
#include "../inc/err.h"
#include "../inc/mem.h"
#include "../inc/heap.h"
#include "../inc/gc.h"
#include "../inc/range.h"
#include "../inc/arr.h"
#include "../inc/int.h"
#include "../inc/float.h"
#include "../inc/bool.h"
#include "../inc/str.h"
#include "../inc/list.h"
#include "../inc/dict.h"
#include "../inc/type.h"
#include "../inc/rand.h"
#include "../inc/eval.h"
#include "../inc/parser.h"
#include "../inc/ast.h"
#include "../inc/env.h"

size_t MAX_INPUT_LINE = 80;
static void eval_expr(ast_expr_t *expr, env_t *env, eval_result_t *result);

static void eval_nil_expr(ast_expr_t *expr, eval_result_t *result) {
  if (TYPEOF(expr) != AST_NIL) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }
  obj_t* obj = (obj_t*) alloc_type(TYPE_NIL, F_NONE);
  result->obj = obj;
}

static void eval_boolean_expr(ast_expr_t *expr, eval_result_t *result) {
  if (TYPEOF(expr) != AST_BOOLEAN) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }
  result->obj = boolean_obj(expr->boolval);
}

static void eval_rand(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  eval_expr(expr, env, result);
  if (TYPEOF(result->obj) != TYPE_INT || result->obj->intval < 1) {
    result->err = ERR_TYPE_POSITIVE_INT_REQUIRED;
    result->obj = nil_obj();
    return;
  }
  result->obj = int_obj(rand32() % result->obj->intval);
}

static void eval_abs(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  eval_expr(expr, env, result);

  static_method m = get_static_method(TYPEOF(result->obj), METHOD_ABS);
  if (m == NULL) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = m(result->obj, NULL);
}

static void eval_type_of(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  eval_expr(expr, env, result);

  if (result->err != ERR_NO_ERROR) return;

  result->obj = string_obj(c_str_to_bytearray(type_names[TYPEOF(result->obj)]));
}

static void eval_to_hex(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  eval_expr(expr, env, result);

  if (TYPEOF(result->obj) != TYPE_INT) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = string_obj(int_to_hex(result->obj->intval));
}

static void eval_to_bin(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  eval_expr(expr, env, result);

  if (TYPEOF(result->obj) != TYPE_INT) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = string_obj(int_to_bin(result->obj->intval));
}

static void eval_int_expr(ast_expr_t *expr, eval_result_t *result) {
  if (TYPEOF(expr) != AST_INT) {
    printf("not int expr!\n");
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }
  result->obj = int_obj(expr->intval);
}

static void eval_float_expr(ast_expr_t *expr, eval_result_t *result) {
  if (TYPEOF(expr) != AST_FLOAT) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }
  result->obj = float_obj(expr->floatval);
}

static void eval_byte_expr(ast_expr_t *expr, eval_result_t *result) {
  if (TYPEOF(expr) != AST_BYTE) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }
  result->obj = byte_obj(expr->byteval);
}

static void eval_list_expr(ast_list_t *list, eval_result_t *result, env_t *env) {
  if (list->es == NULL) {
    result->obj = list_obj(NULL);
    return;
  }

  obj_list_element_t *elem = (obj_list_element_t*) alloc_type(
      TYPE_LIST_ELEM_DATA, F_NONE);
  obj_list_element_t *root_elem = elem;

  ast_expr_list_t *ast_node = list->es;
  while(ast_node != NULL) {
    eval_expr(ast_node->root, env, result);

    if (result->err != ERR_NO_ERROR) {
      result->obj = list_obj(NULL);
      return;
    }

    ast_node = ast_node->next;

    // Link up the nodes on the obj.
    elem->node = result->obj;
    if (ast_node != NULL)  {
      elem->next = (obj_list_element_t*) alloc_type(TYPE_LIST_ELEM_DATA, F_NONE);
    } else {
      elem->next = NULL;
    }
    elem = elem->next;
  }

  result->obj = list_obj(root_elem);
}

static void eval_dict_expr(ast_dict_t *expr, eval_result_t *result, env_t *env) {
  if (expr->kv == NULL) {
    result->obj = dict_obj();
    return;
  }

  obj_t *dict = dict_obj();
  dict_init(dict, DICT_INIT_BUCKETS);

  ast_expr_kv_list_t *kv = expr->kv;
  while(kv != NULL) {
    eval_expr(kv->k, env, result);
    if (result->err != ERR_NO_ERROR) {
      printf("Failed to evaluate %s for key.\n", type_names[TYPEOF(kv->k)]);
      result->obj = nil_obj();
      return;
    }
    obj_t *k = result->obj;

    eval_expr(kv->v, env, result);
    if (result->err != ERR_NO_ERROR) {
      printf("Failed to evaluate %s for value.\n", type_names[TYPEOF(kv->v)]);
      result->obj = nil_obj();
      return;
    }
    obj_t *v = result->obj;

    if ((result->err = dict_put(dict, k, v)) != ERR_NO_ERROR) {
      printf("Failed to put kv in dict!\n");
      return;
    }

    kv = kv->next;
  }

  result->obj = dict;
}

static void _eval_block_expr_in_scope(ast_expr_list_t *block_exprs,
                               eval_result_t *result,
                               env_t *env) {
  ast_expr_list_t *node = block_exprs;
  obj_t *last_obj = nil_obj();

  while (node != NULL) {
    eval_expr(node->root, env, result);
    if (result->err != ERR_NO_ERROR)  return;

    // Unwrap and return any return val.
    if (TYPEOF(result->obj) == TYPE_RETURN_VAL) {
      result->obj = result->obj->return_val;
      return;
    }

    if (TYPEOF(result->obj) == TYPE_BREAK) return;

    if (TYPEOF(result->obj) == TYPE_CONTINUE) break;

    // The last meaningful object in the block is its value.
    if (TYPEOF(result->obj) != TYPE_NOTHING) {
      last_obj = result->obj;
    }
    node = node->next;
  }
  result->obj = last_obj;
}

static void eval_block_expr(ast_expr_list_t *block_exprs, eval_result_t *result, env_t *env) {
  enter_scope(env);
  _eval_block_expr_in_scope(block_exprs, result, env);
  leave_scope(env);
  //gc(env);
}

static void eval_return_expr(ast_expr_list_t *block_exprs, eval_result_t *result, env_t *env) {
  enter_scope(env);
  _eval_block_expr_in_scope(block_exprs, result, env);
  // Wrap the return obj.
  result->obj = return_val(result->obj);
  leave_scope(env);
  //gc(env);
}

/* Wrap the function pointer in an obj. */
static void eval_func_def(ast_func_def_t *func_def, eval_result_t *result, env_t *env) {
  result->obj = func_obj((void *) func_def, env->symbols[env->top]);
}

static void eval_func_call(ast_func_call_t *func_call, eval_result_t *result, env_t *env) {
  eval_expr(func_call->expr, env, result);
  if (result->err != ERR_NO_ERROR) {
    result->err = ERR_FUNCTION_UNDEFINED;
    result->obj = nil_obj();
    return;
  }
  obj_t *obj = result->obj;

  if (TYPEOF(obj) != TYPE_FUNCTION) {
    result->err = ERR_FUNCTION_UNDEFINED;
    result->obj = nil_obj();
    return;
  }

  ast_func_def_t *fn = (ast_func_def_t *) obj->func_def->code;

  /* Push the scope the function was defined in ... */
  env_sym_elem_t *scope = (env_sym_elem_t *) obj->func_def->scope;
  error_t err = push_scope(env, scope);

  if ((result->err = err) != ERR_NO_ERROR) {
    leave_scope(env);
    result->obj = nil_obj();
    return;
  }

  /* ... and create a new scope for the function itself. */
  err = enter_scope(env);
  if ((result->err = err) != ERR_NO_ERROR) {
    result->obj = nil_obj();
    goto done;
  }

  ast_fn_arg_decl_t *argnames = fn->argnames;
  ast_expr_list_t *callargs = func_call->args;

  while (argnames != NULL) {
    if (callargs == NULL) {
      result->err = ERR_WRONG_ARG_COUNT;
      goto done;
    }
    bytearray_t *name = argnames->name;
    eval_expr(callargs->root, env, result);
    error_t err = put_env_shadow(env, name, result->obj, F_NONE);
    if (err != ERR_NO_ERROR) {
      result->err = err;
      goto done;
    }

    argnames = argnames->next;
    callargs = callargs->next;
  }

  _eval_block_expr_in_scope(fn->block_exprs, result, env);

done:
  // Not a typo. There was a push_scope and an enter_scope.
  leave_scope(env);
  leave_scope(env);
}

static void eval_string_expr(ast_expr_t *expr, eval_result_t *result) {
  if (TYPEOF(expr) != AST_STRING) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }
  obj_t* obj = string_obj(expr->bytearray);
  result->obj = obj;
}

static void is_type(obj_t *obj, obj_t *type_obj, eval_result_t *result) {
  if ( !(TYPEOF(type_obj) == TYPE_INT
      || TYPEOF(type_obj) == TYPE_FLOAT
      || TYPEOF(type_obj) == TYPE_BYTE
      || TYPEOF(type_obj) == TYPE_STRING
      || TYPEOF(type_obj) == TYPE_BOOLEAN)) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = boolean_obj(TYPEOF(obj) == TYPEOF(type_obj));
}

static void cast(obj_t *obj, obj_t *type_obj, eval_result_t *result) {
  static_method m = get_static_method(TYPEOF(obj), METHOD_CAST);

  if (m == NULL) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = m(obj, wrap_varargs(1, type_obj));
}

static void list_subscript_assign(obj_t *obj,
                                  ast_expr_t *lhs,
                                  ast_expr_t *rhs,
                                  eval_result_t *result,
                                  env_t *env) {
  eval_expr(lhs->op_args->b, env, result);
  if (result->err != ERR_NO_ERROR) goto error;
  obj_t *offset = result->obj;
  eval_expr(rhs, env, result);
  if (result->err != ERR_NO_ERROR) goto error;
  obj_t *val = result->obj;
  result->obj = list_set(obj, wrap_varargs(2, offset, val));
  return;

error:
  result->obj = nil_obj();
}

static void arr_subscript_assign(obj_t *obj,
                                 ast_expr_t *lhs,
                                 ast_expr_t *rhs,
                                 eval_result_t *result,
                                 env_t *env) {
  eval_expr(lhs->op_args->b, env, result);
  if (result->err != ERR_NO_ERROR) goto error;
  obj_t *offset = result->obj;
  eval_expr(rhs, env, result);
  if (result->err != ERR_NO_ERROR) goto error;
  obj_t *val = result->obj;
  result->obj = arr_set(obj, wrap_varargs(2, offset, val));
  return;

error:
  result->obj = nil_obj();
}

static void dict_subscript_assign(obj_t *obj,
                                  ast_expr_t *lhs,
                                  ast_expr_t *rhs,
                                  eval_result_t *result,
                                  env_t *env) {
  eval_expr(lhs->op_args->b, env, result);
  if (result->err != ERR_NO_ERROR) goto error;
  obj_t *k = result->obj;
  eval_expr(rhs, env, result);
  if (result->err != ERR_NO_ERROR) goto error;
  obj_t *v = result->obj;
  result->obj = dict_obj_put(obj, wrap_varargs(2, k, v));
  return;

error:
  result->obj = nil_obj();

}

static void assign(ast_expr_t *lhs,
                   ast_expr_t *rhs,
                   eval_result_t *result,
                   env_t *env) {

  if (TYPEOF(lhs) == AST_SUBSCRIPT) {
    // TODO this looks like a bug that might find an obj in the env
    // e.g., if we say "foo"[1] = 'c', and env has an obj named foo.
    bytearray_t *name = lhs->op_args->a->bytearray;
    obj_t *obj = get_env(env, name);

    switch(TYPEOF(obj)) {
      case TYPE_LIST:
        list_subscript_assign(obj, lhs, rhs, result, env);
        return;
      case TYPE_STRING:    // fall-through
      case TYPE_BYTEARRAY:
        arr_subscript_assign(obj, lhs, rhs, result, env);
        return;
      case TYPE_DICT:
        dict_subscript_assign(obj, lhs, rhs, result, env);
        return;
      default:
        printf("can't assign to %s\n", type_names[TYPEOF(obj)]);
        result->err = ERR_AST_TYPE_UNHANDLED;
        return;
    }
  }

  if (!(FLAGS(lhs) & F_ENV_ASSIGNABLE)) {
    result->err = ERR_LHS_NOT_ASSIGNABLE;
    return;
  }

  bytearray_t *name = lhs->bytearray;
  error_t error;

  if (TYPEOF(rhs) == AST_FUNCTION_DEF) {
    eval_func_def(rhs->func_def, result, env);
    if (result->err != ERR_NO_ERROR) goto error;
    error = put_env(env, name, result->obj, F_NONE);
    if (error != ERR_NO_ERROR) {
      result->err = error;
      goto error;
    }
    return;
  }

  eval_expr(rhs, env, result);
  if (result->err != ERR_NO_ERROR) goto error;
  result->err = put_env(env, name, result->obj, FLAGS(lhs));
  if (result->err != ERR_NO_ERROR) goto error;

  return;

error:
  result->obj = nil_obj();
}

static void cmp(type_t type, obj_t *a, obj_t *b, eval_result_t *result) {
  static_method m;
  switch (type) {
    case AST_EQ: m = get_static_method(TYPEOF(a), METHOD_EQ); break;
    case AST_NE: m = get_static_method(TYPEOF(a), METHOD_NE); break;
    case AST_LT: m = get_static_method(TYPEOF(a), METHOD_LT); break;
    case AST_GT: m = get_static_method(TYPEOF(a), METHOD_GT); break;
    case AST_LE: m = get_static_method(TYPEOF(a), METHOD_LE); break;
    case AST_GE: m = get_static_method(TYPEOF(a), METHOD_GE); break;
  }

  if (m == NULL) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = m(a, wrap_varargs(1, b));
}

static void member_of(obj_t *a, obj_t *b, eval_result_t *result) {
  static_method m = get_static_method(TYPEOF(b), METHOD_CONTAINS);
  if (m == NULL) {
    result->err = ERR_TYPE_ITERABLE_REQUIRED;
    return;
  }

  result->obj = m(b, wrap_varargs(1, a));
}

static void subscript_of(obj_t *a, obj_t *b, eval_result_t *result) {
  static_method m = get_static_method(TYPEOF(a), METHOD_GET);
  if (m == NULL) {
    result->err = ERR_TYPE_ITERABLE_REQUIRED;
    return;
  }

  result->obj = m(a, wrap_varargs(1, b));
}

static boolean truthy(obj_t *obj) {
  switch(TYPEOF(obj)) {
    case TYPE_NIL: return False;
    case TYPE_INT: return obj->intval != 0;
    case TYPE_FLOAT: return obj->floatval != 0;
    case TYPE_STRING: return obj->bytearray->size > 0;
    case TYPE_BYTE: return obj->byteval != 0x0;
    case TYPE_BOOLEAN: return obj->boolval;
    default:
      printf("Unknown type: %llu\n", TYPEOF(obj));
      return False;
  }
}

static void boolean_and(obj_t *a, obj_t *b, eval_result_t *result) {
  result->obj = boolean_obj(truthy(a) && truthy(b));
}

static void boolean_or(obj_t *a, obj_t *b, eval_result_t *result) {
  result->obj = boolean_obj(truthy(a) || truthy(b));
}

static void negate(obj_t *a, eval_result_t *result) {
  static_method m = get_static_method(TYPEOF(a), METHOD_NEG);
  if (m == NULL) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = m(a, NULL);
}

static void math(obj_t *a, obj_t *b,
                 eval_result_t *result,
                 static_method_ident_t method_id) {
  static_method m = get_static_method(TYPEOF(a), method_id);
  if (m == NULL) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = m(a, wrap_varargs(1, b));
}

static void range(int from_inclusive, int to_inclusive, eval_result_t *result) {
  result->obj = range_obj(from_inclusive, to_inclusive);
}

static void range_step(int from_inclusive,
                       int to_inclusive,
                       int step,
                       eval_result_t *result) {
  result->obj = range_step_obj(from_inclusive, to_inclusive, step);
}

static void apply(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  if (TYPEOF(expr) != AST_APPLY) {
    result->err = ERR_SYNTAX_ERROR;
    return;
  }

  eval_expr(expr->application->receiver, env, result);
  if (result->err != ERR_NO_ERROR) return;

  obj_t *obj = result->obj;

  // Look up the method identifier.
  static_method_ident_t method_id = METHOD_NONE;
  bytearray_t *function_name = expr->application->function_name;
  for (int i = 0; i < sizeof(static_method_names) / sizeof(static_method_names[0]); i++) {
    if (c_str_eq_bytearray(static_method_names[i].name, function_name)) {
      method_id = static_method_names[i].ident;
      goto found;
    }
  }
found:
  if (method_id == METHOD_NONE) {
    result->err = ERR_NO_SUCH_METHOD;
    return;
  }

  static_method method = get_static_method(TYPEOF(obj), method_id);

  if (method == NULL) {
    printf("Method not found for that object\n");
    result->err = ERR_NO_SUCH_METHOD;
    return;
  }

  if (expr->application->args == NULL) {
    // Zero args.
    result->obj = method(obj, NULL);
  } else {
    // Args to eval.
    ast_expr_list_t *args = expr->application->args;
    obj_varargs_t *method_args = (obj_varargs_t*) alloc_type(TYPE_VARIABLE_ARGS, F_NONE);
    obj_varargs_t *method_args_root = method_args;

    while(args != NULL) {
      eval_expr(args->root, env, result);

      method_args->arg = result->obj;
      method_args->next = NULL;
      args = args->next;

      if (args != NULL) {
        obj_varargs_t *next_method_args = (obj_varargs_t*) alloc_type(TYPE_VARIABLE_ARGS, F_NONE);
        method_args->next = next_method_args;
        method_args = next_method_args;
      }
    }

    result->obj = method(obj, method_args_root);
  }
}

static int print_args(ast_expr_list_t *args, eval_result_t *result, env_t *env) {
  int printed = 0;
  ast_expr_list_t *node = (ast_expr_list_t*) args;
  if (TYPEOF(args->root) != AST_EMPTY) {
    while(node != NULL) {
      eval_expr(node->root, env, result);

      if (result->err != ERR_NO_ERROR) {
        result->obj = undef_obj();
        return printed;
      }

      // TODO: probably want to put the whole string result in result->obj.
      // TODO: it needs tests, one way or the other
      printed++;
      switch (TYPEOF(result->obj)) {
        case TYPE_INT: printf("%d ", result->obj->intval); break;
        case TYPE_FLOAT: printf("%f ", (double) result->obj->floatval); break;
        case TYPE_BYTE: printf("%c ", result->obj->byteval); break;
        case TYPE_STRING: printf("%s ", bytearray_to_c_str(result->obj->bytearray)); break;
        case TYPE_BOOLEAN: printf("%s ", result->obj->boolval ? "true" : "false"); break;
        case TYPE_NIL: printf("Nil "); break;
        case TYPE_BYTEARRAY: printf("%s", result->obj->bytearray->data);
        default: printf("?? "); break;
      }
      node = node->next;
    }
  }
  result->obj = no_obj();
  return printed;
}

static void println_args(ast_expr_list_t *args, eval_result_t *result, env_t *env) {
  print_args(args, result, env);
  printf("\n");
}

// For messing about in the repl.
static void eval_read_file(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  eval_expr(expr, env, result);
  if (TYPEOF(result->obj) != TYPE_STRING) {
    result->err = ERR_BAD_FILENAME;
    result->obj = nil_obj();
    return;
  }

  FILE *f = fopen(bytearray_to_c_str(result->obj->bytearray), "rb");
  if (!f) {
    result->err = ERR_BAD_FILENAME;
    result->obj = nil_obj();
    return;
  }

  long file_len;
  fseek(f, 0, SEEK_END);
  file_len = ftell(f);
  obj_t *buffer = bytearray_obj(file_len, NULL);

  fseek(f, 0, SEEK_SET);
  fread (buffer->bytearray->data, 1, file_len, f);
  fclose(f);

  result->obj = buffer;
}

static void dump_args(ast_expr_list_t *args, eval_result_t *result, env_t *env) {
  if (args->root == NULL) {
    result->obj = nil_obj();
    return;
  }

  eval_expr(args->root, env, result);
  switch(TYPEOF(result->obj)) {
    case TYPE_BYTE:
      result->obj = byte_dump(result->obj);
      break;
    case TYPE_INT:
      result->obj = int32_dump(result->obj);
      break;
    case TYPE_FLOAT:
      result->obj = float32_dump(result->obj);
      return;
    case TYPE_STRING:
    case TYPE_BYTEARRAY:
      result->obj = arr_dump(result->obj);
      break;
    default:
      printf("Cannot dump object of type %s.\n", type_names[TYPEOF(result->obj)]);
      result->obj = nil_obj();
  }
}

static void readln_input(eval_result_t *result) {
  char* s = mem_alloc(MAX_INPUT_LINE);
  if (getline(&s, &MAX_INPUT_LINE, stdin) == -1) {
    result->err = ERR_INPUT_STREAM_ERROR;
  }

  // Trim trailing newline.
  int end = (int) c_str_len(s) - 1;
  while (end >= 0 && s[end] == '\n') s[end--] = '\0';

  result->obj = string_obj(c_str_to_bytearray(s));
}

static void resolve_callable_expr(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  if (TYPEOF(expr) != AST_RESERVED_CALLABLE) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  ast_expr_list_t *args = expr->reserved_callable->es;

  switch (expr->reserved_callable->type) {
    case AST_CALL_TYPE_OF:
      eval_type_of(args->root, result, env);
      break;
    case AST_CALL_TO_HEX:
      eval_to_hex(args->root, result, env);
      break;
    case AST_CALL_TO_BIN:
      eval_to_bin(args->root, result, env);
      break;
    case AST_CALL_DUMP:
      dump_args(args, result, env);
      break;
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
    case AST_CALL_READ:
      eval_read_file(args->root, result, env);
      break;
    case AST_CALL_RAND:
      eval_rand(args->root, result, env);
      break;
    case AST_CALL_ABS:
      eval_abs(args->root, result, env);
      break;
    case AST_CALL_GC:
      gc(env);
      break;
    case AST_CALL_MEM:
      show_heap();
      break;
    case AST_CALL_ENV:
      show_env(env);
      break;
    default:
      goto error;
      break;
  }

  return;

error:
  result->err = ERR_AST_TYPE_UNHANDLED;
  result->obj = undef_obj();
}

static void eval_do_while_loop(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  eval_result_t *cond_r = (eval_result_t*) alloc_type(EVAL_RESULT, F_NONE);

  ast_expr_t *pred = expr->do_while_loop->pred;
  ast_expr_t *cond = expr->do_while_loop->cond;

  result->obj = nil_obj();
  result->err = ERR_NO_ERROR;

  for(;;) {
    eval_expr(pred, env, result);
    if (result->err != ERR_NO_ERROR) {
      result->obj = undef_obj();
      return;
    }

    if (TYPEOF(result->obj) == TYPE_BREAK) {
      result->obj = nil_obj();
      return;
    } else if(TYPEOF(result->obj) == TYPE_CONTINUE) {
      result->obj = nil_obj();
    }

    eval_expr(cond, env, cond_r);
    if (cond_r->err != ERR_NO_ERROR) {
      result->err = cond_r->err;
      result->obj = undef_obj();
      return;
    }

    if (!truthy(cond_r->obj)) return;
  }
}

static void eval_while_loop(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  eval_result_t *cond_r = (eval_result_t*) alloc_type(EVAL_RESULT, F_NONE);

  result->obj = nil_obj();
  result->err = ERR_NO_ERROR;

  ast_expr_t *cond = expr->while_loop->cond;
  ast_expr_t *pred = expr->while_loop->pred;

  enter_scope(env);

  for(;;) {
    eval_expr(cond, env, cond_r);
    if (cond_r->err != ERR_NO_ERROR) {
      result->err = cond_r->err;
      result->obj = undef_obj();
      goto done;
    }

    if (!truthy(cond_r->obj)) goto done;

    eval_expr(pred, env, result);
    if (result->err != ERR_NO_ERROR) {
      result->obj = undef_obj();
      goto done;
    }

    if (TYPEOF(result->obj) == TYPE_BREAK) {
      result->obj = nil_obj();
      goto done;
    } else if (TYPEOF(result->obj) == TYPE_CONTINUE) {
      result->obj = nil_obj();
    }
  }

done:
  leave_scope(env);
}

static void eval_for_loop(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  obj_t *result_obj = undef_obj();
  result->obj = result_obj;

  ast_expr_t *pred = expr->for_loop->pred;

  size_t orig_expr = (size_t) expr;

  // Local name for the variable holding each element.
  // This is what gets updated as we iterate.
  bytearray_t *elem_name = ((ast_expr_t*) expr->for_loop->elem)->bytearray;

  // The object whose elements we want to iterate over.
  // Evaluate the iterable expression to get it.
  eval_result_t *iter_r = (eval_result_t*) alloc_type(EVAL_RESULT, F_NONE);

  eval_expr(expr->for_loop->iterable, env, iter_r);
  if ((result->err = iter_r->err) != ERR_NO_ERROR) {
    printf("obj %llu err %d\n", ((gc_header_t*) iter_r->obj)->type, iter_r->err);
    goto error;
  }
  obj_t *iter_obj = iter_r->obj;

  // An iterator object that points to said object and maintains
  // state as we iterate.
  static_method get_iterator = get_static_method(TYPEOF(iter_obj), METHOD_ITERATOR);
  if (get_iterator == NULL) {
    result->err = ERR_NO_SUCH_METHOD;
    printf("No iterator!\n");
    goto error;
  }
  obj_iter_t *iter = get_iterator(iter_obj, NULL)->iterator;

  // Push a new scope and store the index variable.
  // We will mutate this variable with each iteration through the loop.
  enter_scope(env);
  obj_t *next_elem = iter->next(iter);

  // The actual iteration. Done when we encounter Nil as a sentinel.
  while(TYPEOF(next_elem) != TYPE_NIL) {
    // Not mutable in user code.
    assert(orig_expr == (size_t) expr);
    put_env(env, elem_name, next_elem, F_ENV_OVERWRITE);

    eval_expr(pred, env, result);

    if (result->err != ERR_NO_ERROR) goto error;

    if (TYPEOF(result->obj) == TYPE_BREAK) goto done;

    if (TYPEOF(result->obj) == TYPE_CONTINUE) continue;

    result_obj = result->obj;

    next_elem = iter->next(iter);
  }

done:
  leave_scope(env);
  result->obj = result_obj;
  return;

error:
  printf("Leaving scope on error %d\n", result->err);
  leave_scope(env);
  result->obj = undef_obj();
}

static void eval_expr(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  result->err = ERR_NO_ERROR;

  switch(TYPEOF(expr)) {
    case AST_EMPTY: {
      result->obj = no_obj();
      break;
    }
    case AST_IS: {
      eval_expr(expr->cast_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->cast_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      is_type(o1, o2, result);
      break;
    }
    case AST_CAST: {
      eval_expr(expr->cast_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->cast_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      cast(o1, o2, result);
      break;
    }
    case AST_ADD: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      math(o1, o2, result, METHOD_ADD);
      break;
    }
    case AST_SUB: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      math(o1, o2, result, METHOD_SUB);
      break;
    }
    case AST_MUL: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      math(o1, o2, result, METHOD_MUL);
      break;
    }
    case AST_DIV: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      math(o1, o2, result, METHOD_DIV);
      break;
    }
    case AST_MOD: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      math(o1, o2, result, METHOD_MOD);
      break;
    }
    case AST_AND: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      boolean_and(o1, o2, result);
      break;
    }
    case AST_OR: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      boolean_or(o1, o2, result);
      break;
    }
    case AST_NOT: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      result->obj = boolean_obj(truthy(result->obj) == True ? False : True);
      break;
    }
    case AST_NEGATE: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      negate(result->obj, result);
      break;
    }
    case AST_BITWISE_NOT: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      math(result->obj, NULL, result, METHOD_BITWISE_NOT);
      break;
    }
    case AST_BITWISE_SHL: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      math(o1, o2, result, METHOD_BITWISE_SHL);
      break;
    }
    case AST_BITWISE_SHR: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      math(o1, o2, result, METHOD_BITWISE_SHR);
      break;
    }
    case AST_BITWISE_OR: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      math(o1, o2, result, METHOD_BITWISE_OR);
      break;
    }
    case AST_BITWISE_XOR: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      math(o1, o2, result, METHOD_BITWISE_XOR);
      break;
    }
    case AST_BITWISE_AND: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      math(o1, o2, result, METHOD_BITWISE_AND);
      break;
    }
    case AST_GT:
    case AST_GE:
    case AST_LT:
    case AST_LE:
    case AST_NE:
    case AST_EQ: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      cmp(TYPEOF(expr), o1, o2, result);
      break;
    }
    case AST_ASSIGN: {
      assign(expr->op_args->a, expr->op_args->b, result, env);
      break;
    }
    case AST_IN: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      member_of(o1, o2, result);
      break;
    }
    case AST_SUBSCRIPT: {
      eval_expr(expr->op_args->a, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o1 = result->obj;
      eval_expr(expr->op_args->b, env, result);
      if (result->err != ERR_NO_ERROR) return;
      obj_t *o2 = result->obj;
      subscript_of(o1, o2, result);
      break;
    }
    case AST_RANGE: {
      eval_expr(expr->range->from, env, result);
      obj_t *o1 = result->obj;
      if (result->err != ERR_NO_ERROR) return;
      if (TYPEOF(o1) != TYPE_INT) result->err = ERR_TYPE_INT_REQUIRED;
      eval_expr(expr->range->to, env, result);
      obj_t *o2 = result->obj;
      if (result->err != ERR_NO_ERROR) return;
      if (TYPEOF(o2) != TYPE_INT) result->err = ERR_TYPE_INT_REQUIRED;
      if (expr->range->step != NULL) {
        eval_expr(expr->range->step, env, result);
        obj_t *o3 = result->obj;
        if (result->err != ERR_NO_ERROR) return;
        if (TYPEOF(o3) != TYPE_INT) result->err = ERR_TYPE_INT_REQUIRED;
        range_step(o1->intval,
                   o2->intval,
                   o3->intval,
                   result);
      } else {
        range(o1->intval, o2->intval, result);
      }
      break;
    }
    case AST_FUNCTION_RETURN: {
      eval_return_expr(expr->func_return_values, result, env);
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
      break;
    case AST_FLOAT:
      eval_float_expr(expr, result);
      break;
    case AST_BYTE:
      eval_byte_expr(expr, result);
      break;
    case AST_STRING:
      eval_string_expr(expr, result);
      break;
    case AST_IDENT: {
      bytearray_t *name = expr->bytearray;
      obj_t *obj = get_env(env, name);
      if (TYPEOF(obj) == TYPE_UNDEF) {
        result->err = ERR_ENV_SYMBOL_UNDEFINED;
        return;
      }
      result->obj = obj;
      break;
    }
    case AST_BYTEARRAY_DECL: {
      if (TYPEOF((obj_t*)expr->range->from) != AST_INT) {
        result->err = ERR_TYPE_INT_REQUIRED;
        return;
      }
      eval_expr(expr->range->from, env, result);
      if (result->err != ERR_NO_ERROR) return;
      result->obj = bytearray_obj(result->obj->intval, NULL);
      break;
    }
    case AST_LIST: {
      eval_list_expr(expr->list, result, env);
      if (result->err != ERR_NO_ERROR) return;
      break;
    }
    case AST_DICT: {
      eval_dict_expr(expr->dict, result, env);
      if (result->err != ERR_NO_ERROR) return;
      break;
    }
    case AST_BLOCK: {
      eval_block_expr(expr->block_exprs, result, env);
      if (result->err != ERR_NO_ERROR) return;
      break;
    }
    case AST_FUNCTION_DEF: {
      eval_func_def(expr->func_def, result, env);
      if (result->err != ERR_NO_ERROR) return;
      break;
    }
    case AST_FUNCTION_CALL: {
      eval_func_call(expr->func_call, result, env);
      if (result->err != ERR_NO_ERROR) return;
      break;
    }
    case AST_RESERVED_CALLABLE: {
      resolve_callable_expr(expr, env, result);
      if (result->err != ERR_NO_ERROR) return;
      break;
    }
    case AST_APPLY: {
      apply(expr, result, env);
      if (result->err != ERR_NO_ERROR) return;
      break;
    }
    case AST_DELETE: {
      error_t error = del_env(env, expr->bytearray);
      if (error != ERR_NO_ERROR) {
        result->err = error;
        return;
      }
      result->obj = nil_obj();
      break;
    }
    case AST_IF_THEN: {
      eval_expr(expr->if_then_args->cond, env, result);
      if (result->err != ERR_NO_ERROR) return;
      // If cond ...
      if (truthy(result->obj)) {
        eval_expr(expr->if_then_else_args->pred, env, result);
        if (result->err != ERR_NO_ERROR) return;
        // ... then result.
        break;
      }
      result->obj = nil_obj();
      break;
    }
    case AST_IF_THEN_ELSE: {
      eval_expr(expr->if_then_else_args->cond, env, result);
      if (result->err != ERR_NO_ERROR) return;
      // If cond ...
      if (truthy(result->obj)) {
        eval_expr(expr->if_then_else_args->pred, env, result);
        // ... then result ...
        break;
      } else {
        eval_expr(expr->if_then_else_args->else_pred, env, result);
        // ... else other result.
        break;
      }
      result->obj = nil_obj();
      break;
    }
    case AST_DO_WHILE_LOOP:
      eval_do_while_loop(expr, env, result);
      break;
    case AST_WHILE_LOOP:
      eval_while_loop(expr, env, result);
      break;
    case AST_FOR_LOOP:
      eval_for_loop(expr, env, result);
      break;
    case AST_BREAK:
      result->obj = break_obj();
      break;
    case AST_CONTINUE:
      result->obj = continue_obj();
      break;
    default:
      printf("Can't eval: %s\n", type_names[TYPEOF(expr)]);
      result->err = ERR_EVAL_UNHANDLED_OBJECT;
      break;
  }
}

void eval(env_t *env, const char *input, eval_result_t *result) {
  ast_expr_t *ast = ast_empty();
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(input, ast, parse_result);

  result->err = parse_result->err;
  result->depth = parse_result->depth;

  if (parse_result->err != ERR_NO_ERROR) {
    result->obj = no_obj();
    return;
  }

#ifdef DEBUG
  pretty_print(ast);
#endif

  eval_expr(ast, env, result);
}

