#include <stdio.h>
#include "../inc/int.h"
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
  obj_t *eq = int_eq(obj, args);
  return boolean_obj((eq->boolval == True) ? False : True);
}

static_method get_int_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return int_hash;
    case METHOD_EQ: return int_eq;
    case METHOD_NE: return int_ne;
    default: return NULL;
  }
}

