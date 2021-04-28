#include <stdio.h>
#include "../inc/float.h"
#include "../inc/obj.h"

obj_t *float_hash(obj_t *obj, obj_method_args_t *args) {
  // 32-bit float is its own 32-bit hash value.
  uint32_t *ip = (uint32_t *) &(obj->floatval);
  return int_obj(*ip);
}

static_method get_float_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return float_hash;
    default: return NULL;
  }
}

