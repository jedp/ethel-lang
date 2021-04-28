#ifndef __BOOL_H
#define __BOOL_H

#include "obj.h"

obj_t *bool_hash(obj_t *obj, obj_method_args_t *args);

static_method get_bool_static_method(static_method_ident_t method_id);

#endif

