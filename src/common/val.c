#include <stdio.h>
#include <string.h>
#include "../obj/obj.h"
#include "val.h"

bool val_eq(val_t *a, val_t *b) {
    // TODO test b is of primitive type
    switch (a->type) {
        case VAL_TYPE_BOOL:
            return (a->as.boolval == b->as.boolval);
        case VAL_TYPE_BYTE:
            return (a->as.byteval == b->as.byteval);
        case VAL_TYPE_CHAR:
            return (a->as.charval == b->as.charval);
        case VAL_TYPE_UINT:
        case VAL_TYPE_FLOAT:
            return (a->as.uintval == b->as.uintval);
        case VAL_TYPE_INT:
            return (a->as.intval == b->as.intval);
        case VAL_TYPE_OBJ: {
            if (a->type == OBJ_TYPE_STRING && b->type == OBJ_TYPE_STRING) {
                obj_str_t *a_str = AS_OBJ_STR(a);
                obj_str_t *b_str = AS_OBJ_STR(b);
                if (a_str->length != b_str->length)
                    return false;
                return memcmp(a_str->chars, b_str->chars, a_str->length) == 0;
            }
            return false;
        }

        case VAL_TYPE_ADDR:
            // TODO
            return false;
        default:
            printf("Unsupported type for primitive comparison: %d\n", a->type);
            return false;
    }
}
