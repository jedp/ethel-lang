#ifndef __RANGE_H
#define __RANGE_H

#include "obj.h"

/*
 * Return the number of elements in the range.
 *
 * range_length(1..4) -> 4
 * range_length(4..1) -> 4
 * range_length(1..1) -> 1
 * range_length(0..-3) -> 4
 */
obj_t *range_length(obj_t *range_obj, obj_method_args_t *args);

/* Return true if the first arg is an int and is enclosed by the range. */
obj_t *range_contains(obj_t *range_obj, obj_method_args_t *args);

/* Return the element in the range at the given offset. */
obj_t *range_get(obj_t *range_obj, obj_method_args_t *args);

obj_t *range_random_choice(obj_t *range_obj, obj_method_args_t *args);

static_method get_range_static_method(static_method_ident_t method_id);

#endif

