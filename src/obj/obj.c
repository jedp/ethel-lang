#include <string.h>
#include "../mem/mem.h"
#include "obj.h"

obj_t *obj_new(uint32_t size, obj_type_t type) {
    obj_t *obj = (obj_t *) mem_realloc(NULL, 0, size);
    obj->type = type;
    return obj;
}
