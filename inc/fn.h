#ifndef __FN_H
#define __FN_H

#include "obj.h"

obj_t *fn_to_string(obj_t *obj, obj_method_args_t *args);

static_method get_fn_static_method(static_method_ident_t method_id);

#endif

