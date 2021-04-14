#include <stdio.h>
#include "../inc/arr.h"

obj_t *arr_size(obj_t *arr_obj, obj_method_args_t /* Ignored */ *args) {
  return int_obj(arr_obj->bytearray->size);
}

obj_t *arr_get(obj_t *arr_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to get()\n");
    return nil_obj();
  }
  // Get first arg as int offset.
  obj_t *arg = args->arg;
  if (arg->type != TYPE_INT) {
    return nil_obj();
  }
  int i = arg->intval;

  if (i >= arr_obj->bytearray->size) {
    printf("Out of bounds\n");
    return nil_obj();
  }

  return byte_obj(arr_obj->bytearray->data[i]);
}

static_method get_arr_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_LENGTH: return arr_size;
    case METHOD_GET: return arr_get;
    default: return NULL;
  }
}

