#include<stdio.h>
#include "../inc/bool.h"
#include "../inc/obj.h"

obj_t *bool_hash(obj_t *obj, obj_method_args_t *args) {
  // 32-bit int is its own 32-bit hash.
  return int_obj((uint32_t) obj->boolval);
}

static_method get_bool_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return bool_hash;
    default: return NULL;
  }
}

