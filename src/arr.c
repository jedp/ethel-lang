#include <stdio.h>
#include "../inc/arr.h"

static byte obj_to_byte(obj_t *obj) {
  switch(obj->type) {
    case TYPE_BYTE:
      return obj->byteval;
    case TYPE_INT: {
      if (obj->intval > 255 || obj->intval < -127) return 0xFF;
      return (byte) obj->intval;
    }
    default:
      printf("Don't know how to convert %s to byte.\n", obj_type_names[obj->type]);
      return 0x0;
  }
}

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

  if (i < 0 || i >= arr_obj->bytearray->size) {
    printf("Out of bounds\n");
    return nil_obj();
  }

  return byte_obj(arr_obj->bytearray->data[i]);
}

obj_t *arr_set(obj_t *arr_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to get()\n");
    return nil_obj();
  }
  // Get first arg as int offset.
  obj_t *a = args->arg;
  if (a->type != TYPE_INT) {
    printf("Int offset required.\n");
    return nil_obj();
  }
  int i = a->intval;
  if (i < 0 || i >= arr_obj->bytearray->size) {
    printf("Out of bounds\n");
    return nil_obj();
  }

  // Get second arg as byte to set.
  obj_t *b = args->next->arg;
  if (b == NULL) {
    printf("Null arg for value.\n");
    return nil_obj();
  }
 
  byte val = obj_to_byte(b);
  arr_obj->bytearray->data[i] = val;
  return byte_obj(val);
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

