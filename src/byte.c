#include <stdio.h>
#include "../inc/byte.h"
#include "../inc/obj.h"
#include "../inc/str.h"

obj_t *byte_hash(obj_t *obj, obj_method_args_t *args) {
  // 8-bit int is its own 32-bit hash.
  return int_obj((uint32_t) obj->byteval);
}

obj_t *byte_eq(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_BYTE:
      return boolean_obj((obj->byteval == arg->byteval) ? True : False);
    case TYPE_INT:
      return boolean_obj(
          ((uint8_t) obj->byteval == (uint32_t) arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
            obj_type_names[TYPE_BYTE], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *byte_ne(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_BYTE && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           obj_type_names[TYPE_FLOAT], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *eq = byte_eq(obj, args);
  return boolean_obj(eq->boolval == True ? False : True);
}

obj_t *byte_lt(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_BYTE:
      return boolean_obj((obj->byteval < arg->byteval) ? True : False);
    case TYPE_INT:
      return boolean_obj(
          ((uint8_t) obj->byteval < (uint32_t) arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
            obj_type_names[TYPE_BYTE], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *byte_gt(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_BYTE:
      return boolean_obj((obj->byteval > arg->byteval) ? True : False);
    case TYPE_INT:
      return boolean_obj(
          ((uint8_t) obj->byteval > (uint32_t) arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
            obj_type_names[TYPE_BYTE], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *byte_ge(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_BYTE && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           obj_type_names[TYPE_FLOAT], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *lt = byte_lt(obj, args);
  return boolean_obj(lt->boolval == True ? False : True);
}

obj_t *byte_le(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_BYTE && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           obj_type_names[TYPE_FLOAT], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *gt = byte_gt(obj, args);
  return boolean_obj(gt->boolval == True ? False : True);
}

obj_t *byte_as(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *type_arg = args->arg;

  switch (type_arg->type) {
    case TYPE_BYTE: return obj;
    case TYPE_INT: return int_obj((unsigned int) obj->byteval);
    case TYPE_BOOLEAN: return boolean_obj((obj->byteval == 't') ? 1 : 0);
    case TYPE_STRING: {
      bytearray_t *a = bytearray_alloc(1);
      a->data[0] = obj->byteval;
      return string_obj(a);
    }
  }

  printf("Cannot cast %s to %s.\n",
          obj_type_names[TYPE_BYTE], obj_type_names[type_arg->type]);
  return nil_obj();

}

static_method get_byte_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return byte_hash;
    case METHOD_EQ: return byte_eq;
    case METHOD_NE: return byte_ne;
    case METHOD_GT: return byte_gt;
    case METHOD_LT: return byte_lt;
    case METHOD_GE: return byte_ge;
    case METHOD_LE: return byte_le;
    case METHOD_CAST: return byte_as;
    default: return NULL;
  }
}

