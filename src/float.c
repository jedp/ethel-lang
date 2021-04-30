#include <stdio.h>
#include "../inc/float.h"
#include "../inc/str.h"
#include "../inc/obj.h"

obj_t *float_hash(obj_t *obj, obj_method_args_t *args) {
  // 32-bit float is its own 32-bit hash value.
  uint32_t *ip = (uint32_t *) &(obj->floatval);
  return int_obj(*ip);
}

obj_t *float_abs(obj_t *obj, obj_method_args_t *args) {
  return float_obj((obj->floatval < 0) ? 1-obj->floatval : obj->floatval);
}

obj_t *float_eq(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_FLOAT:
      return boolean_obj((obj->floatval == arg->floatval) ? True : False);
    case TYPE_INT:
      return boolean_obj((obj->floatval == arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
             obj_type_names[TYPE_FLOAT], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *float_ne(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_FLOAT && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           obj_type_names[TYPE_FLOAT], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *eq = float_eq(obj, args);
  return boolean_obj(eq->boolval == True ? False : True);
}

obj_t *float_lt(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_FLOAT:
      return boolean_obj((obj->floatval < arg->floatval) ? True : False);
    case TYPE_INT:
      return boolean_obj((obj->floatval < arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
            obj_type_names[TYPE_FLOAT], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *float_gt(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_FLOAT:
      return boolean_obj((obj->floatval > arg->floatval) ? True : False);
    case TYPE_INT:
      return boolean_obj((obj->floatval > arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
            obj_type_names[TYPE_FLOAT], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *float_le(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_FLOAT && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           obj_type_names[TYPE_FLOAT], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *gt = float_gt(obj, args);
  return boolean_obj((gt->boolval == True) ? False : True);
}

obj_t *float_ge(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_FLOAT && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           obj_type_names[TYPE_FLOAT], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *lt = float_lt(obj, args);
  return boolean_obj((lt->boolval == True) ? False : True);
}

obj_t *float_as(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  if (arg->type == TYPE_FLOAT) {
    return obj;
  }

  if (arg->type == TYPE_INT) {
    return int_obj((int) obj->floatval);
  }

  if (arg->type == TYPE_STRING) {
    return string_obj(float_to_str(obj->floatval));
  }

  if (arg->type == TYPE_BOOLEAN) {
    return boolean_obj(truthy(obj) ? True : False);
  }

  printf("Cannot cast %s to %s.\n",
          obj_type_names[TYPE_FLOAT], obj_type_names[arg->type]);
  return nil_obj();
}

static_method get_float_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return float_hash;
    case METHOD_ABS: return float_abs;
    case METHOD_EQ: return float_eq;
    case METHOD_NE: return float_ne;
    case METHOD_LT: return float_lt;
    case METHOD_GT: return float_gt;
    case METHOD_LE: return float_le;
    case METHOD_GE: return float_ge;
    case METHOD_CAST: return float_as;
    default: return NULL;
  }
}

