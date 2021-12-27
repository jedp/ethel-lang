#ifndef __TYPE_H
#define __TYPE_H

#include "def.h"
#include "obj.h"

#define TYPEOF(x) (((gc_header_t*) x)->type)
#define NAMEOF(x) ((TYPEOF(x) < TYPE_MAX) ? type_names[TYPEOF(x)] : "???")
#define FLAGS(x) (((gc_header_t*) x)->flags)

static_method get_static_method(type_t obj_type,
                                static_method_ident_t method_id);

#endif

