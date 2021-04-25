#ifndef __RANGE_H
#define __RANGE_H

#include "obj.h"

/* Return true if the first arg is an int and is enclosed by the range. */
obj_t *range_contains(obj_t *range_obj, obj_method_args_t *args);

/* Return the element in the range at the given offset. */
obj_t *range_subscript(obj_t *range_obj, obj_method_args_t *args);

static_method get_range_static_method(static_method_ident_t method_id);

#endif

