#ifndef __NUM_H
#define __NUM_H

#include "def.h"
#include "obj.h"

obj_t *num_eq(obj_t *obj, obj_t *other);
obj_t *num_le(obj_t *obj, obj_t *other);
obj_t *num_lt(obj_t *obj, obj_t *other);
obj_t *num_ge(obj_t *obj, obj_t *other);
obj_t *num_gt(obj_t *obj, obj_t *other);
obj_t *num_ne(obj_t *obj, obj_t *other);
obj_t *num_add(obj_t *obj, obj_t *other);
obj_t *num_sub(obj_t *obj, obj_t *other);
obj_t *num_mul(obj_t *obj, obj_t *other);
obj_t *num_div(obj_t *obj, obj_t *other);
obj_t *num_mod(obj_t *obj, obj_t *other);

boolean is_numeric(obj_t *obj);

#endif

