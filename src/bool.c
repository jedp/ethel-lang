#include<stdio.h>
#include "../inc/bool.h"
#include "../inc/obj.h"

obj_t *bool_hash(obj_t *obj, obj_method_args_t *args) {
  // 32-bit int is its own 32-bit hash.
  return int_obj((uint32_t) obj->boolval);
}

obj_t *bool_eq(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  return boolean_obj(obj->boolval == truthy(arg));
}

obj_t *bool_ne(obj_t *obj, obj_method_args_t *args) {
  return boolean_obj(bool_eq(obj, args)->boolval == True ? False : True);
}

static_method get_bool_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return bool_hash;
    case METHOD_EQ: return bool_eq;
    case METHOD_NE: return bool_ne;
    default: return NULL;
  }
}

