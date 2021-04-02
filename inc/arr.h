#ifndef __ARR_H
#define __ARR_H

#include "obj.h"

obj_t *arr_size(obj_t *arr_obj, obj_method_args_t *args);
obj_t *arr_get(obj_t *arr_obj, obj_method_args_t *args);

static_method get_arr_static_method(static_method_ident_t method_id);

#endif

