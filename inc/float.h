#ifndef __FLOAT_H
#define __FLOAT_H

#include "obj.h"

/* Return the integer hash of the float value. */
obj_t *float_hash(obj_t *obj, obj_method_args_t *args);

static_method get_float_static_method(static_method_ident_t method_id);

#endif

