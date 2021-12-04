#ifndef __FN_H
#define __FN_H

#include "obj.h"

obj_t *fn_to_string(obj_t *obj, obj_t *args_obj);

static_method get_fn_static_method(static_method_ident_t method_id);

#endif

