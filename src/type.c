#include <stdio.h>
#include "../inc/obj.h"
#include "../inc/type.h"
#include "../inc/int.h"
#include "../inc/float.h"
#include "../inc/bool.h"
#include "../inc/arr.h"
#include "../inc/str.h"
#include "../inc/list.h"

static_method get_static_method(obj_type_t obj_type,
                                static_method_ident_t method_id) {
  switch (obj_type) {
    case TYPE_INT:       return get_int_static_method(method_id);
    case TYPE_FLOAT:     return get_float_static_method(method_id);
    case TYPE_BOOLEAN:   return get_bool_static_method(method_id);
    case TYPE_BYTEARRAY: return get_arr_static_method(method_id);
    case TYPE_LIST:      return get_list_static_method(method_id);
    case TYPE_STRING:    return get_str_static_method(method_id);
    default:
      printf("Method %s not found for %s\n",
             static_method_names[method_id].name,
             obj_type_names[obj_type]);
      return NULL;
  }
}

