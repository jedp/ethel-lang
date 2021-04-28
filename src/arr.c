#include <stdio.h>
#include "../inc/math.h"
#include "../inc/mem.h"
#include "../inc/obj.h"
#include "../inc/arr.h"

static boolean _eq(bytearray_t *a, bytearray_t *b) {
  if (a->size != b->size) return False;

  for (dim_t i = 0; i < a->size; i++) {
    if (a->data[i] != b->data[i]) {
      return False;
    }
  }

  return True;
}

static obj_t *_arr_slice(obj_t *arr_obj, int start, int end) {
  if (end > arr_obj->bytearray->size) end = arr_obj->bytearray->size;

  if (end < 0 ||
      start < 0 ||
      end < start) {
    printf("Bad range for slice\n");
    return bytearray_obj(0, NULL);
  }

  dim_t len = end - start;
  obj_t *new = bytearray_obj(len, NULL);
  for (int i = 0; i < len; i++) {
    new->bytearray->data[i] = arr_obj->bytearray->data[start+i];
  }

  return new;
}

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

obj_t *arr_hash(obj_t *arr_obj, obj_method_args_t /* Ignored */ *args) {
  uint32_t temp;
  byte b;

  if (arr_obj->bytearray->size == 0) {
    return nil_obj();
  }

  /*
   * FNV (Fowler, Noll, Vo), the non-cryptographic hash function FNV-1a
   * for 32-bit hashes returning a 32-bit integer.
   */
  temp = FNV32Basis;
  dim_t i = 0;
  while (i < arr_obj->bytearray->size) {
    b = arr_obj->bytearray->data[i];
    temp = FNV32Prime * (temp ^ b);
    i++;
  }

  return int_obj(temp);
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

obj_t *arr_eq(obj_t *arr_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to eq()\n");
    return nil_obj();
  }

  obj_t *other = args->arg;
  if (other->type != TYPE_STRING &&
      other->type != TYPE_BYTEARRAY) {
    return nil_obj();
  }

  return boolean_obj(_eq(arr_obj->bytearray, other->bytearray));
}

obj_t *arr_ne(obj_t *arr_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to ne()\n");
    return nil_obj();
  }

  obj_t *other = args->arg;
  if (other->type != TYPE_STRING &&
      other->type != TYPE_BYTEARRAY) {
    return nil_obj();
  }

  return boolean_obj(!_eq(arr_obj->bytearray, other->bytearray));
}

obj_t *arr_slice(obj_t *arr_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to slice()\n");
    return nil_obj();
  }

  obj_t *start_arg = args->arg;
  if (start_arg->type != TYPE_INT) return nil_obj();

  obj_t *end_arg = args->next->arg;
  if (end_arg->type != TYPE_INT)  return nil_obj();

  return _arr_slice(arr_obj, start_arg->intval, end_arg->intval);
}

static_method get_arr_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return arr_hash;
    case METHOD_LENGTH: return arr_size;
    case METHOD_GET: return arr_get;
    case METHOD_CONTAINS: return arr_contains;
    case METHOD_EQ: return arr_eq;
    case METHOD_NE: return arr_ne;
    case METHOD_SLICE: return arr_slice;
    default: return NULL;
  }
}

