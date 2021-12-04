#include <stdio.h>
#include "../inc/err.h"
#include "../inc/mem.h"
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
eval_result_t *eval_expr(ast_expr_t *expr, env_t *env);

static void eval_nil_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_NIL) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }
  obj_t* obj = mem_alloc(sizeof(obj_t));
  obj->type = TYPE_NIL;
  result->obj = obj;
}

static void eval_boolean_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_BOOLEAN) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }
  result->obj = boolean_obj(expr->boolval);
}

static void eval_rand(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  eval_result_t *r = eval_expr(expr, env);
  if (r->obj->type != TYPE_INT || r->obj->intval < 1) {
    result->err = ERR_TYPE_POSITIVE_INT_REQUIRED;
    result->obj = nil_obj();
    return;
  }
  result->obj = int_obj(rand32() % r->obj->intval);
}

static void eval_abs(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  eval_result_t *r = eval_expr(expr, env);

  static_method m = get_static_method(r->obj->type, METHOD_ABS);
  if (m == NULL) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = m(r->obj, NULL);
}

static void eval_type_of(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  eval_result_t *r = eval_expr(expr, env);

  if (r->err != ERR_NO_ERROR) {
    result->err = r->err;
    return;
  }

  // TODO fix when we have user-defined types
  result->obj = string_obj(c_str_to_bytearray(type_names[r->obj->type]));
}

static void eval_to_hex(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  eval_result_t *r = eval_expr(expr, env);

  if (r->obj->type != TYPE_INT) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = string_obj(int_to_hex(r->obj->intval));
}

static void eval_to_bin(ast_expr_t *expr, eval_result_t *result, env_t *env) {
  eval_result_t *r = eval_expr(expr, env);

  if (r->obj->type != TYPE_INT) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = string_obj(int_to_bin(r->obj->intval));
}

static void eval_int_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_INT) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }
  result->obj = int_obj(expr->intval);
}

static void eval_float_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_FLOAT) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }
  result->obj = float_obj(expr->floatval);
}

static void eval_byte_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_BYTE) {
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

  obj_list_element_t *elem = mem_alloc(sizeof(obj_list_element_t));
  mark_traceable(elem, TYPE_LIST_ELEM_DATA);
  obj_list_element_t *root_elem = elem;

  ast_expr_list_t *ast_node = list->es;
  while(ast_node != NULL) {
    eval_result_t *r = eval_expr(ast_node->root, env);

    if (r->err != ERR_NO_ERROR) {
      result->err = r->err;
      result->obj = list_obj(NULL);
      return;
    }

    ast_node = ast_node->next;

    // Link up the nodes on the obj.
    elem->node = r->obj;
    if (ast_node != NULL)  {
      elem->next = mem_alloc(sizeof(obj_list_element_t));
      mark_traceable(elem->next, TYPE_LIST_ELEM_DATA);
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
  eval_result_t *r;
  while(kv != NULL) {
    r = eval_expr(kv->k, env);
    if ((result->err = r->err) != ERR_NO_ERROR) {
      printf("Failed to evaluate %s for key.\n", type_names[kv->k->type]);
      result->obj = nil_obj();
      return;
    }
    obj_t *k = r->obj;

    r = eval_expr(kv->v, env);
    if ((result->err = r->err) != ERR_NO_ERROR) {
      printf("Failed to evaluate %s for value.\n", type_names[kv->v->type]);
      result->obj = nil_obj();
      return;
    }
    obj_t *v = r->obj;

    if ((result->err = dict_put(dict, k, v)) != ERR_NO_ERROR) {
      mem_free(dict);
      dict = NULL;
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
  result->obj = nil_obj();
  while (node != NULL) {
    eval_result_t *r = eval_expr(node->root, env);
    if ((result->err = r->err) != ERR_NO_ERROR) {
      return;
    }

    // Unwrap and return any return val.
    if (r->obj->type == TYPE_RETURN_VAL) {
      result->obj = r->obj->return_val;
      return;
    }

    if (r->obj->type == TYPE_BREAK) {
      result->obj = break_obj();
      return;
    }

    if (r->obj->type == TYPE_CONTINUE) {
      result->obj = continue_obj();
      return;
    }

    // The last meaningful object in the block is its value.
    // TODO destructuring or something for return vals
    if (r->obj->type != TYPE_NOTHING) {
      result->obj = r->obj;
    }
    node = node->next;
  }
}

static void eval_block_expr(ast_expr_list_t *block_exprs, eval_result_t *result, env_t *env) {
  enter_scope(env);
  _eval_block_expr_in_scope(block_exprs, result, env);
  leave_scope(env);
}

static void eval_return_expr(ast_expr_list_t *block_exprs, eval_result_t *result, env_t *env) {
  enter_scope(env);
  _eval_block_expr_in_scope(block_exprs, result, env);
  // Wrap the return obj.
  result->obj = return_val(result->obj);
  leave_scope(env);
}

/* Wrap the function pointer in an obj. */
static void eval_func_def(ast_func_def_t *func_def, eval_result_t *result, env_t *env) {
  result->obj = func_obj((void *) func_def, env->symbols[env->top]);
}

static void eval_func_call(ast_func_call_t *func_call, eval_result_t *result, env_t *env) {
  eval_result_t *r = eval_expr(func_call->expr, env);
  if (r->err != ERR_NO_ERROR) {
    result->err = ERR_FUNCTION_UNDEFINED;
    result->obj = nil_obj();
    return;
  }
  obj_t *obj = r->obj;

  if (obj->type != TYPE_FUNC_PTR) {
    result->err = ERR_FUNCTION_UNDEFINED;
    result->obj = nil_obj();
    return;
  }

  ast_func_def_t *fn = (ast_func_def_t *) obj->func_def->code;
  env_sym_t *scope = (env_sym_t *) obj->func_def->scope;

  /* Push the scope the function was defined in ... */
  error_t err = push_scope(env, scope);
  if ((result->err = err) != ERR_NO_ERROR) {
    leave_scope(env);
    result->obj = nil_obj();
    return;
  }

  /* ... and create a new scope for the function itself. */
  err = enter_scope(env);
  if ((result->err = err) != ERR_NO_ERROR) {
    // Not a typo. There was a push_scope and an enter_scope.
    leave_scope(env);
    leave_scope(env);
    result->obj = nil_obj();
    return;
  }

  ast_fn_arg_decl_t *argnames = fn->argnames;
  ast_expr_list_t *callargs = func_call->args;
  while (argnames != NULL) {
    if (callargs == NULL) {
      result->err = ERR_WRONG_ARG_COUNT;
      // Not a typo. There was a push_scope and an enter_scope.
      leave_scope(env);
      leave_scope(env);
      return;
    }
    bytearray_t *name = argnames->name;
    eval_result_t *r = eval_expr(callargs->root, env);
    error_t err = put_env_shadow(env, string_obj(name), r->obj, F_NONE);
    if (err != ERR_NO_ERROR) {
      result->err = err;
      // Not a typo. There was a push_scope and an enter_scope.
      leave_scope(env);
      leave_scope(env);
      return;
    }

    argnames = argnames->next;
    callargs = callargs->next;
  }

  _eval_block_expr_in_scope(fn->block_exprs, result, env);

  // Not a typo. There was a push_scope and an enter_scope.
  leave_scope(env);
  leave_scope(env);
  return;
}

static void eval_string_expr(ast_expr_t *expr, eval_result_t *result) {
  if (expr->type != AST_STRING) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }
  obj_t* obj = string_obj(expr->bytearray);
  result->obj = obj;
}

static void is_type(obj_t *obj, obj_t *type_obj, eval_result_t *result) {
  if ( !(type_obj->type == TYPE_INT
      || type_obj->type == TYPE_FLOAT
      || type_obj->type == TYPE_BYTE
      || type_obj->type == TYPE_STRING
      || type_obj->type == TYPE_BOOLEAN)) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = boolean_obj(obj->type == type_obj->type);
}

static void cast(obj_t *obj, obj_t *type_obj, eval_result_t *result) {
  static_method m = get_static_method(obj->type, METHOD_CAST);

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
  eval_result_t *r1 = eval_expr(lhs->op_args->b, env);
  if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
  obj_t *offset = r1->obj;
  eval_result_t *r2 = eval_expr(rhs, env);
  if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
  obj_t *val = r2->obj;
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
  eval_result_t *r1 = eval_expr(lhs->op_args->b, env);
  if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
  obj_t *offset = r1->obj;
  eval_result_t *r2 = eval_expr(rhs, env);
  if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
  obj_t *val = r2->obj;
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
  eval_result_t *r1 = eval_expr(lhs->op_args->b, env);
  if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
  obj_t *k = r1->obj;
  eval_result_t *r2 = eval_expr(rhs, env);
  if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
  obj_t *v = r2->obj;
  result->obj = dict_obj_put(obj, wrap_varargs(2, k, v));
  return;

error:
  result->obj = nil_obj();

}

static void assign(ast_expr_t *lhs,
                   ast_expr_t *rhs,
                   eval_result_t *result,
                   env_t *env) {

  if (lhs->type == AST_SUBSCRIPT) {
    // TODO this looks like a bug that might find an obj in the env
    // e.g., if we say "foo"[1] = 'c', and env has an obj named foo.
    bytearray_t *name = lhs->op_args->a->bytearray;
    obj_t *obj = get_env(env, string_obj(name));

    switch(obj->type) {
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
        result->err = ERR_AST_TYPE_UNHANDLED;
        return;
    }
  }

  if (!(lhs->flags & F_ASSIGNABLE)) {
    result->err = ERR_LHS_NOT_ASSIGNABLE;
    return;
  }

  bytearray_t *name = lhs->bytearray;
  error_t error;

  if (rhs->type == AST_FUNCTION_DEF) {
    eval_func_def(rhs->func_def, result, env);
    if (result->err != ERR_NO_ERROR) goto error;
    error = put_env(env, string_obj(name), result->obj, F_NONE);
    if (error != ERR_NO_ERROR) {
      result->err = error;
      goto error;
    }
    return;
  }

  eval_result_t *r = eval_expr(rhs, env);
  if ((result->err = r->err) != ERR_NO_ERROR) goto error;
  error = put_env(env, string_obj(name), r->obj, lhs->flags);
  if (error != ERR_NO_ERROR) {
    result->err = error;
    goto error;
  }
  result->obj = r->obj;

  result->err = error;
  if (result->err != ERR_NO_ERROR) goto error;
  return;

error:
  result->obj = nil_obj();
}

static void cmp(ast_type_t type, obj_t *a, obj_t *b, eval_result_t *result) {
  static_method m;
  switch (type) {
    case AST_EQ: m = get_static_method(a->type, METHOD_EQ); break;
    case AST_NE: m = get_static_method(a->type, METHOD_NE); break;
    case AST_LT: m = get_static_method(a->type, METHOD_LT); break;
    case AST_GT: m = get_static_method(a->type, METHOD_GT); break;
    case AST_LE: m = get_static_method(a->type, METHOD_LE); break;
    case AST_GE: m = get_static_method(a->type, METHOD_GE); break;
  }

  if (m == NULL) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = m(a, wrap_varargs(1, b));
}

static void member_of(obj_t *a, obj_t *b, eval_result_t *result) {
  static_method m = get_static_method(b->type, METHOD_CONTAINS);
  if (m == NULL) {
    result->err = ERR_TYPE_ITERABLE_REQUIRED;
    return;
  }

  result->obj = m(b, wrap_varargs(1, a));
}

static void subscript_of(obj_t *a, obj_t *b, eval_result_t *result) {
  static_method m = get_static_method(a->type, METHOD_GET);
  if (m == NULL) {
    result->err = ERR_TYPE_ITERABLE_REQUIRED;
    return;
  }

  result->obj = m(a, wrap_varargs(1, b));
}

static boolean truthy(obj_t *obj) {
  switch(obj->type) {
    case TYPE_NIL: return False;
    case TYPE_INT: return obj->intval != 0;
    case TYPE_FLOAT: return obj->floatval != 0;
    case TYPE_STRING: return obj->bytearray->size > 0;
    case TYPE_BYTE: return obj->byteval != 0x0;
    case TYPE_BOOLEAN: return obj->boolval;
    default:
      printf("Unknown type: %d\n", obj->type);
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
  static_method m = get_static_method(a->type, METHOD_NEG);
  if (m == NULL) {
    result->err = ERR_EVAL_TYPE_ERROR;
    return;
  }

  result->obj = m(a, NULL);
}

static void math(obj_t *a, obj_t *b,
                 eval_result_t *result,
                 static_method_ident_t method_id) {
  static_method m = get_static_method(a->type, method_id);
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
  if (expr->type != AST_APPLY) {
    result->err = ERR_SYNTAX_ERROR;
    return;
  }

  eval_result_t *receiver = eval_expr(expr->application->receiver, env);
  if ((result->err = receiver->err) != ERR_NO_ERROR) {
    return;
  }

  obj_t *obj = receiver->obj;

  // Look up the method identifier.
  static_method_ident_t method_id = METHOD_NONE;
  bytearray_t *member_name = expr->application->member_name;
  for (int i = 0; i < sizeof(static_method_names) / sizeof(static_method_names[0]); i++) {
    if (c_str_eq_bytearray(static_method_names[i].name, member_name)) {
      method_id = static_method_names[i].ident;
      goto found;
    }
  }
found:
  if (method_id == METHOD_NONE) {
    result->err = ERR_NO_SUCH_METHOD;
    return;
  }

  static_method method = get_static_method(obj->type, method_id);

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
    obj_method_args_t *method_args = mem_alloc(sizeof(obj_method_args_t));
    obj_method_args_t *method_args_root = method_args;

    while(args != NULL) {
      eval_result_t *r = eval_expr(args->root, env);

      method_args->arg = r->obj;
      method_args->next = NULL;
      args = args->next;

      if (args != NULL) {
        obj_method_args_t *next_method_args = mem_alloc(sizeof(obj_method_args_t));
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
  if (args->root->type != AST_EMPTY) {
    while(node != NULL) {
      eval_result_t *r = eval_expr(node->root, env);

      if (r->err != ERR_NO_ERROR) {
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
        case TYPE_BYTE: printf("%c ", r->obj->byteval); break;
        case TYPE_STRING: printf("%s ", bytearray_to_c_str(r->obj->bytearray)); break;
        case TYPE_BOOLEAN: printf("%s ", r->obj->boolval ? "true" : "false"); break;
        case TYPE_NIL: printf("Nil "); break;
        case TYPE_BYTEARRAY: printf("%s", r->obj->bytearray->data);
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
  eval_result_t *r = eval_expr(expr, env);
  if (r->obj->type != TYPE_STRING) {
    result->err = ERR_BAD_FILENAME;
    result->obj = nil_obj();
    return;
  }

  FILE *f = fopen(bytearray_to_c_str(r->obj->bytearray), "rb");
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

  eval_result_t *r = eval_expr(args->root, env);
  switch(r->obj->type) {
    case TYPE_BYTE:
      result->obj = byte_dump(r->obj);
      break;
    case TYPE_INT:
      result->obj = int32_dump(r->obj);
      break;
    case TYPE_FLOAT:
      result->obj = float32_dump(r->obj);
      return;
    case TYPE_STRING:
    case TYPE_BYTEARRAY:
      result->obj = arr_dump(r->obj);
      break;
    default:
      printf("Cannot dump object of type %s.\n", type_names[r->obj->type]);
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
  mem_free(s);
  s = NULL;
}

static void resolve_callable_expr(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  if (expr->type != AST_RESERVED_CALLABLE) {
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
  eval_result_t *cond;
  eval_result_t *r;
  result->obj = nil_obj();
  result->err = ERR_NO_ERROR;

  for(;;) {
    r = eval_expr(expr->do_while_loop->pred, env);
    if (r->err != ERR_NO_ERROR) {
      result->err = r->err;
      result->obj = undef_obj();
      return;
    }

    if (r->obj->type == TYPE_BREAK) {
      result->obj = nil_obj();
      return;
    } else if(r->obj->type == TYPE_CONTINUE) {
      result->obj = nil_obj();
    } else {
      result->obj = r->obj;
    }

    cond = eval_expr(expr->do_while_loop->cond, env);
    if (cond->err != ERR_NO_ERROR) {
      result->err = cond->err;
      result->obj = undef_obj();
      return;
    }

    if (!truthy(cond->obj)) return;
  }
}

static void eval_while_loop(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  eval_result_t *cond;
  eval_result_t *r;
  result->obj = nil_obj();
  result->err = ERR_NO_ERROR;

  for(;;) {
    cond = eval_expr(expr->while_loop->cond, env);
    if (cond->err != ERR_NO_ERROR) {
      result->err = cond->err;
      result->obj = undef_obj();
      return;
    }

    if (!truthy(cond->obj)) return;

    r = eval_expr(expr->while_loop->pred, env);
    if (r->err != ERR_NO_ERROR) {
      result->err = r->err;
      result->obj = undef_obj();
      return;
    }

    if (r->obj->type == TYPE_BREAK) {
      result->obj = nil_obj();
      return;
    } else if (r->obj->type == TYPE_CONTINUE) {
      result->obj = nil_obj();
    } else {
      result->obj = r->obj;
    }
  }
}

static void eval_for_loop(ast_expr_t *expr, env_t *env, eval_result_t *result) {
  eval_result_t *r = mem_alloc(sizeof(eval_result_t));
  r->obj = undef_obj();

  // Local name for the variable holding each element.
  // This is what gets updated as we iterate.
  bytearray_t *elem_name = ((ast_expr_t*) expr->for_loop->elem)->bytearray;

  // The object whose elements we want to iterate over.
  // Evaluate the iterable expression to get it.
  eval_result_t *iter_r = eval_expr(expr->for_loop->iterable, env);
  if ((result->err = iter_r->err) != ERR_NO_ERROR) goto error;
  obj_t *iter_obj = iter_r->obj;

  // An iterator object that points to said object and maintains
  // state as we iterate.
  static_method get_iterator = get_static_method(iter_obj->type, METHOD_ITERATOR);
  if (get_iterator == NULL) {
    r->err = ERR_NO_SUCH_METHOD;
    goto error;
  }
  obj_iter_t *iter = get_iterator(iter_obj, NULL)->iterator;

  // Push a new scope and store the index variable.
  // We will mutate this variable with each iteration through the loop.
  enter_scope(env);
  obj_t *next_elem = iter->next(iter);

  // The actual iteration. Done when we encounter Nil as a sentinel.
  while(next_elem->type != TYPE_NIL) {
    // Not mutable in user code.
    put_env(env, string_obj(elem_name), next_elem, F_OVERWRITE);

    r = eval_expr(expr->for_loop->pred, env);
    if ((result->err = r->err) != ERR_NO_ERROR) {
      goto error;
    }

    if (r->obj->type == TYPE_BREAK) {
      r->obj = nil_obj();
      goto done;
    }

    if (r->obj->type == TYPE_CONTINUE) {
      r->obj = nil_obj();
    }

    next_elem = iter->next(iter);
  }

done:
  result->obj = r->obj;
  leave_scope(env);
  mem_free(r);
  r = NULL;
  return;

error:
  printf("leaving scope on error\n");
  leave_scope(env);
  mem_free(r);
  r = NULL;
  result->obj = undef_obj();
}

eval_result_t *eval_expr(ast_expr_t *expr, env_t *env) {
    eval_result_t *result = mem_alloc(sizeof(eval_result_t));
    result->err = ERR_NO_ERROR;

    switch(expr->type) {
        case AST_EMPTY: {
            result->obj = no_obj();
            break;
        }
        case AST_IS: {
            eval_result_t *r1 = eval_expr(expr->cast_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->cast_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            is_type(r1->obj, r2->obj, result);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_CAST: {
            eval_result_t *r1 = eval_expr(expr->cast_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->cast_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            cast(r1->obj, r2->obj, result);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_ADD: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            math(r1->obj, r2->obj, result, METHOD_ADD);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_SUB: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            math(r1->obj, r2->obj, result, METHOD_SUB);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_MUL: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            math(r1->obj, r2->obj, result, METHOD_MUL);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_DIV: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            math(r1->obj, r2->obj, result, METHOD_DIV);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_MOD: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            math(r1->obj, r2->obj, result, METHOD_MOD);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_AND: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            boolean_and(r1->obj, r2->obj, result); 
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_OR: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            boolean_or(r1->obj, r2->obj, result); 
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_NOT: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            result->obj = boolean_obj(truthy(r1->obj) == True ? False : True);
            break;
        }
        case AST_NEGATE: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            negate(r1->obj, result);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_BITWISE_NOT: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            math(r1->obj, NULL, result, METHOD_BITWISE_NOT);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_BITWISE_SHL: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            math(r1->obj, r2->obj, result, METHOD_BITWISE_SHL);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_BITWISE_SHR: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            math(r1->obj, r2->obj, result, METHOD_BITWISE_SHR);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_BITWISE_OR: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            math(r1->obj, r2->obj, result, METHOD_BITWISE_OR);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_BITWISE_XOR: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            math(r1->obj, r2->obj, result, METHOD_BITWISE_XOR);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_BITWISE_AND: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            math(r1->obj, r2->obj, result, METHOD_BITWISE_AND);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_GT:
        case AST_GE:
        case AST_LT:
        case AST_LE:
        case AST_NE:
        case AST_EQ: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            cmp(expr->type, r1->obj, r2->obj, result); 
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_ASSIGN: {
          assign(expr->op_args->a, expr->op_args->b, result, env);
          break;
        }
        case AST_IN: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            member_of(r1->obj, r2->obj, result);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_SUBSCRIPT: {
            eval_result_t *r1 = eval_expr(expr->op_args->a, env);
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->op_args->b, env);
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            subscript_of(r1->obj, r2->obj, result);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_RANGE: {
            eval_result_t *r1 = eval_expr(expr->range->from, env);
            if (r1->obj->type != AST_INT) result->err = ERR_TYPE_INT_REQUIRED;
            if ((result->err = r1->err) != ERR_NO_ERROR) goto error;
            eval_result_t *r2 = eval_expr(expr->range->to, env);
            if (r2->obj->type != AST_INT) result->err = ERR_TYPE_INT_REQUIRED;
            if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
            if (expr->range->step != NULL) {
              eval_result_t *r3 = eval_expr(expr->range->step, env);
              if (r2->obj->type != AST_INT) result->err = ERR_TYPE_INT_REQUIRED;
              if ((result->err = r2->err) != ERR_NO_ERROR) goto error;
              range_step(r1->obj->intval,
                         r2->obj->intval,
                         r3->obj->intval,
                         result);
            } else {
              range(r1->obj->intval, r2->obj->intval, result);
            }
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        }
        case AST_FUNCTION_RETURN: {
          eval_return_expr(expr->func_return_values, result, env);
          if (result->err != ERR_NO_ERROR) goto error;
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
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        case AST_FLOAT:
            eval_float_expr(expr, result);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        case AST_BYTE:
            eval_byte_expr(expr, result);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        case AST_STRING:
            eval_string_expr(expr, result);
            if (result->err != ERR_NO_ERROR) goto error;
            break;
        case AST_IDENT: {
            bytearray_t *name = expr->bytearray;
            obj_t *obj = get_env(env, string_obj(name));
            if (obj->type == TYPE_UNDEF) {
              result->err = ERR_ENV_SYMBOL_UNDEFINED;
              goto error;
            }
            result->obj = obj;
            break;
        }
        case AST_BYTEARRAY_DECL: {
          if (((obj_t*)expr->range->from)->type != AST_INT) {
            result->err = ERR_TYPE_INT_REQUIRED; goto error;
          }
          eval_result_t *size = eval_expr(expr->range->from, env);
          if ((result->err = size->err) != ERR_NO_ERROR) goto error;
          result->obj = bytearray_obj(size->obj->intval, NULL);
          break;
        }
        case AST_LIST: {
          eval_list_expr(expr->list, result, env);
          if (result->err != ERR_NO_ERROR) goto error;
          break;
        }
        case AST_DICT: {
          eval_dict_expr(expr->dict, result, env);
          if (result->err != ERR_NO_ERROR) goto error;
          break;
        }
        case AST_BLOCK: {
          eval_block_expr(expr->block_exprs, result, env);
          if (result->err != ERR_NO_ERROR) goto error;
          break;
        }
        case AST_FUNCTION_DEF: {
          eval_func_def(expr->func_def, result, env);
          if (result->err != ERR_NO_ERROR) goto error;
          break;
        }
        case AST_FUNCTION_CALL: {
          eval_func_call(expr->func_call, result, env);
          if (result->err != ERR_NO_ERROR) goto error;
          break;
        }
        case AST_RESERVED_CALLABLE: {
          resolve_callable_expr(expr, env, result);
          if (result->err != ERR_NO_ERROR) goto error;
          break;
        }
        case AST_APPLY: {
          apply(expr, result, env);
          if (result->err != ERR_NO_ERROR) goto error;
          break;
        }
        case AST_DELETE: {
          error_t error = del_env(env, string_obj(expr->bytearray));
          if (error != ERR_NO_ERROR) {
            result->err = error;
            goto error;
          }
          result->obj = nil_obj();
          break;
        }
        case AST_IF_THEN: {
          eval_result_t *if_val = eval_expr(expr->if_then_args->cond, env);
          result->err = if_val->err;
          if (result->err != ERR_NO_ERROR) goto error;
          if (truthy(if_val->obj)) {
            eval_result_t *then_val = eval_expr(expr->if_then_else_args->pred, env);
            result->err = then_val->err;
            if (result->err != ERR_NO_ERROR) goto error;
            result->obj = then_val->obj;
            break;
          }
          result->obj = nil_obj();
          break;
        }
        case AST_IF_THEN_ELSE: {
          eval_result_t *if_val = eval_expr(expr->if_then_else_args->cond, env);
          if ((result->err = if_val->err) != ERR_NO_ERROR) goto error;
          if (truthy(if_val->obj)) {
            eval_result_t *then_val = eval_expr(expr->if_then_else_args->pred, env);
            if ((result->err = then_val->err) != ERR_NO_ERROR) goto error;
            result->obj = then_val->obj;
            break;
          } else {
            eval_result_t *else_val = eval_expr(expr->if_then_else_args->else_pred, env);
            if ((result->err = else_val->err) != ERR_NO_ERROR) goto error;
            result->obj = else_val->obj;
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
          result->err = ERR_EVAL_UNHANDLED_OBJECT;
          break;
    }

error:
    return result;
}

eval_result_t *eval(env_t *env, char *input) {
  ast_expr_t *ast = ast_empty();
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(input, ast, parse_result);

  eval_result_t *r = mem_alloc(sizeof(eval_result_t));
  r->err = parse_result->err;
  r->depth = parse_result->depth;

  if (parse_result->err != ERR_NO_ERROR) {
    r->obj = no_obj();
    return r;
  }

#ifdef DEBUG
    pretty_print(ast);
#endif

  mem_free(r);
  r = NULL;
  return eval_expr(ast, env);
}

