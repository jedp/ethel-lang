#include <stdio.h>
#include "../inc/obj.h"
#include "../inc/type.h"
#include "../inc/byte.h"
#include "../inc/int.h"
#include "../inc/float.h"
#include "../inc/bool.h"
#include "../inc/arr.h"
#include "../inc/str.h"
#include "../inc/list.h"
#include "../inc/dict.h"
#include "../inc/range.h"
#include "../inc/fn.h"

static_method get_static_method(type_t obj_type,
                                static_method_ident_t method_id) {
    switch (obj_type) {
        case TYPE_BYTE:
            return get_byte_static_method(method_id);
        case TYPE_INT:
            return get_int_static_method(method_id);
        case TYPE_FLOAT:
            return get_float_static_method(method_id);
        case TYPE_BOOLEAN:
            return get_bool_static_method(method_id);
        case TYPE_BYTEARRAY:
            return get_arr_static_method(method_id);
        case TYPE_LIST:
            return get_list_static_method(method_id);
        case TYPE_DICT:
            return get_dict_static_method(method_id);
        case TYPE_STRING:
            return get_str_static_method(method_id);
        case TYPE_RANGE:
            return get_range_static_method(method_id);
        case TYPE_FUNCTION:
            return get_fn_static_method(method_id);
        default:
            return NULL;
    }
}
