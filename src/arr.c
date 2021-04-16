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

obj_t *arr_contains(obj_t *arr_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to contains()\n");
    return nil_obj();
  }

  obj_t *arg = args->arg;
  if (arg->type != TYPE_INT &&
      arg->type != TYPE_BYTE) {
    return nil_obj();
  }

  if (arg->type == TYPE_INT && arg->intval > 255) {
    printf("Integer too large for byte");
    return nil_obj();
  }

  byte b;
  if (arg->type == TYPE_BYTE) {
    b = arg->byteval;
  } else if (arg->type == TYPE_INT) {
    b = (byte) arg->intval & 0xff;
  } else {
    printf("This can't happen");
    return nil_obj();
  }

  for (dim_t i = 0; i < arr_obj->bytearray->size; i++) {
    if (arr_obj->bytearray->data[i] == b) {
      return boolean_obj(True);
    }
  }

  return boolean_obj(False);
}

static_method get_arr_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_LENGTH: return arr_size;
    case METHOD_GET: return arr_get;
    case METHOD_CONTAINS: return arr_contains;
    default: return NULL;
  }
}

