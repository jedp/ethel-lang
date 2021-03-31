#ifndef __OBJ_STR_H
#define __OBJ_STR_H

#include "obj.h"

obj_t *str_len(obj_t *str_obj, obj_method_args_t *args);
obj_t *str_eq(obj_t *str_obj, obj_method_args_t *args);

static_method get_str_static_method(static_method_ident_t method_id);

#endif

