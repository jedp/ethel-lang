#include <stdio.h>
#include "../inc/mem.h"
#include "../inc/math.h"
#include "../inc/int.h"
#include "../inc/str.h"
#include "../inc/obj.h"

obj_t *int_hash(obj_t *obj, obj_method_args_t *args) {
  // 32-bit int is its own 32-bit hash.
  return int_obj(obj->intval);
}

obj_t *int_eq(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_INT:
      return boolean_obj((obj->intval == arg->intval) ? True : False);
    case TYPE_FLOAT:
      return boolean_obj((obj->intval == arg->floatval) ? True : False);
    case TYPE_BYTE:
      return boolean_obj((obj->intval == (int) arg->byteval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
              obj_type_names[TYPE_INT], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *int_ne(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_INT &&
      arg->type != TYPE_FLOAT &&
      arg->type != TYPE_BYTE) {
    printf("Cannot compare for equality between %s and %s.\n",
            obj_type_names[TYPE_INT], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *eq = int_eq(obj, args);
  return boolean_obj((eq->boolval == True) ? False : True);
}

obj_t *int_lt(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_INT:
      return boolean_obj((obj->intval < arg->intval) ? True : False);
    case TYPE_FLOAT:
      return boolean_obj((obj->intval < arg->floatval) ? True : False);
    case TYPE_BYTE:
      return boolean_obj(
          ((uint32_t) obj->intval < (uint32_t) arg->byteval) ? True : False);
    default:
      printf("Cannot compare %s and %s.\n",
             obj_type_names[TYPE_INT], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *int_gt(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_INT:
      return boolean_obj((obj->intval > arg->intval) ? True : False);
    case TYPE_FLOAT:
      return boolean_obj((obj->intval > arg->floatval) ? True : False);
    case TYPE_BYTE:
      return boolean_obj(
          ((uint32_t) obj->intval > (uint32_t) arg->byteval) ? True : False);
    default:
      printf("Cannot compare %s and %s.\n",
             obj_type_names[TYPE_INT], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *int_le(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_INT &&
      arg->type != TYPE_FLOAT &&
      arg->type != TYPE_BYTE) {
    printf("Cannot compare for equality between %s and %s.\n",
            obj_type_names[TYPE_INT], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *gt = int_gt(obj, args);
  return boolean_obj((gt->boolval == True) ? False : True);
}

obj_t *int_ge(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_INT &&
      arg->type != TYPE_FLOAT &&
      arg->type != TYPE_BYTE) {
    printf("Cannot compare for equality between %s and %s.\n",
            obj_type_names[TYPE_INT], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *lt = int_lt(obj, args);
  return boolean_obj((lt->boolval == True) ? False : True);
}

obj_t *int_as(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *type_arg = args->arg;

  switch (type_arg->type) {
    case TYPE_INT: return obj;
    case TYPE_FLOAT: return float_obj((float) obj->intval);
    case TYPE_BYTE: return byte_obj((uint32_t) obj->intval & 0xff);
    case TYPE_STRING: return string_obj(int_to_str(obj->intval));
    case TYPE_BOOLEAN: return boolean_obj((obj->intval != 0) ? True : False);
    default:
      printf("Cannot cast %s to type %s.\n",
             obj_type_names[TYPE_INT], obj_type_names[type_arg->type]);
      return boolean_obj(False);
  }
}

obj_t *int_abs(obj_t *obj, obj_method_args_t /* Ignored */ *args) {
  return int_obj(abs(obj->intval));
}


static_method get_int_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return int_hash;
    case METHOD_EQ: return int_eq;
    case METHOD_NE: return int_ne;
    case METHOD_LT: return int_lt;
    case METHOD_LE: return int_le;
    case METHOD_GT: return int_gt;
    case METHOD_GE: return int_ge;
    case METHOD_CAST: return int_as;
    case METHOD_ABS: return int_abs;
    default: return NULL;
  }
}

