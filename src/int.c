#include <stdio.h>
#include "../inc/int.h"
#include "../inc/obj.h"

obj_t *int_hash(obj_t *obj, obj_method_args_t *args) {
  // 32-bit int is its own 32-bit hash.
  return int_obj(obj->intval);
}

static_method get_int_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return int_hash;
    default: return NULL;
  }
}

