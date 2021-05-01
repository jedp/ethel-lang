#ifndef __DICT_H
#define __DICT_H

#include "def.h"
#include "err.h"
#include "obj.h"

error_t dict_init(obj_t *dict_obj, uint32_t buckets);
obj_t *dict_get(obj_t *dict_obj, obj_t *k);
error_t dict_put(obj_t *dict_obj, obj_t *k, obj_t *v);
error_t dict_remove(obj_t *dict_obj, obj_t *k);
boolean dict_contains(obj_t *dict_obj, obj_t *k);

static_method get_dict_static_method(static_method_ident_t method_id);

#endif

