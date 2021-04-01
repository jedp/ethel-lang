#include <stdlib.h>
#include <string.h>
#include "../inc/obj.h"
#include "../inc/str.h"

obj_t *str_len(obj_t *str_obj, obj_method_args_t *args) {
  return int_obj(strlen(str_obj->stringval));
}

obj_t *str_eq(obj_t *str_obj, obj_method_args_t *args) {
  return boolean_obj(false);
}

static_method get_str_static_method(static_method_ident_t method_id) {
  switch(method_id) {
    case METHOD_LENGTH: return str_len;
    case METHOD_EQUALS: return str_eq;
    default: return NULL;
  }
}

