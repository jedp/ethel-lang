#include <stdio.h>
#include "../inc/fn.h"
#include "../inc/str.h"

obj_t *fn_to_string(obj_t *obj, obj_varargs_t *args) {
    (void) obj;
    (void) args;

    return string_obj(c_str_to_bytearray("<Function>"));
}

static_method get_fn_static_method(static_method_ident_t method_id) {
    if (method_id == METHOD_TO_STRING) {
        return fn_to_string;
    }

    return NULL;
}
