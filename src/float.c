#include <stdio.h>
#include "../inc/float.h"
#include "../inc/obj.h"

obj_t *float_hash(obj_t *obj, obj_method_args_t *args) {
  // 32-bit float is its own 32-bit hash value.
  uint32_t *ip = (uint32_t *) &(obj->floatval);
  return int_obj(*ip);
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
  obj_t *eq = float_eq(obj, args);
  return boolean_obj(eq->boolval == True ? False : True);
}

static_method get_float_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return float_hash;
    case METHOD_EQ: return float_eq;
    case METHOD_NE: return float_ne;
    default: return NULL;
  }
}

