#include<stdio.h>
#include "../inc/type.h"
#include "../inc/bool.h"
#include "../inc/str.h"
#include "../inc/obj.h"

obj_t *bool_hash(obj_t *obj, obj_varargs_t *args) {
  // 32-bit int is its own 32-bit hash.
  return int_obj((uint32_t) obj->boolval);
}

obj_t *bool_copy(obj_t *obj, obj_varargs_t *args) {
  return boolean_obj(obj->boolval);
}

obj_t *bool_to_string(obj_t *obj, obj_varargs_t *args) {
  return string_obj(c_str_to_bytearray(
        (obj->boolval == True) ? "true" : "false"));
}

obj_t *bool_eq(obj_t *obj, obj_varargs_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  if (TYPEOF(arg) != TYPE_BOOLEAN) return boolean_obj(False);
  return boolean_obj(obj->boolval == arg->boolval);
}

obj_t *bool_ne(obj_t *obj, obj_varargs_t *args) {
  return boolean_obj(bool_eq(obj, args)->boolval == True ? False : True);
}

obj_t *bool_as(obj_t *obj, obj_varargs_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *type_arg = args->arg;

  switch (TYPEOF(type_arg)) {
    case TYPE_BOOLEAN: return obj;
    case TYPE_INT: return int_obj((obj->boolval == True) ? 1 : 0);
    case TYPE_BYTE: return byte_obj((obj->boolval == True) ? 't' : 'f');
    case TYPE_STRING: return string_obj(c_str_to_bytearray(
                            (obj->boolval == True) ? "true":  "false"));
    default:
      printf("Cannot cast %s to type %s.\n",
             type_names[TYPE_BOOLEAN], type_names[TYPEOF(type_arg)]);
      return boolean_obj(False);
  }
}

static_method get_bool_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return bool_hash;
    case METHOD_COPY: return bool_copy;
    case METHOD_TO_STRING: return bool_to_string;
    case METHOD_EQ: return bool_eq;
    case METHOD_NE: return bool_ne;
    case METHOD_CAST: return bool_as;
    default: return NULL;
  }
}

