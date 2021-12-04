#ifndef __DICT_H
#define __DICT_H

#include "def.h"
#include "err.h"
#include "obj.h"

// Internal dict methods. Exposed for testing.
error_t dict_init(obj_t *obj, uint32_t buckets);
obj_t *dict_get(obj_t *obj, obj_t *k);
error_t dict_put(obj_t *obj, obj_t *k, obj_t *v);
obj_t *dict_remove(obj_t *obj, obj_t *k);
boolean dict_contains(obj_t *obj, obj_t *k);

// Static methods for actual use.
obj_t *dict_obj_get(obj_t *obj, obj_t *args_obj);
obj_t *dict_obj_put(obj_t *obj, obj_t *args_obj);
obj_t *dict_obj_keys(obj_t *obj, obj_t *args_obj);
obj_t *dict_obj_len(obj_t *obj, obj_t *args_obj);
obj_t *dict_obj_remove(obj_t *obj, obj_t *args_obj);
obj_t *dict_obj_iterator(obj_t *obj, obj_t *args_obj);

static_method get_dict_static_method(static_method_ident_t method_id);

#endif

