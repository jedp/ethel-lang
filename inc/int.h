#ifndef __INT_H
#define __INT_H

#include "obj.h"

/* Return the integer hash of the integer. */
obj_t *int_hash(obj_t *obj, obj_method_args_t *args);

static_method get_int_static_method(static_method_ident_t method_id);

#endif

