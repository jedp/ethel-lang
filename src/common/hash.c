#include <stdio.h>
#include <stdlib.h>
#include "hash.h"

static uint32_t obj_bytearray_hash(const uint8_t *bytes, uint32_t length) {
    uint32_t temp;
    uint8_t b;

    if (length == 0) {
        return 0;
    }

    /*
     * FNV (Fowler, Noll, Vo), the non-cryptographic hash function FNV-1a
     * for 32-bit hashes returning a 32-bit integer.
     */
    temp = FNV32Basis;
    size_t i = 0;
    while (i < length) {
        b = bytes[i];
        temp = FNV32Prime * (temp ^ b);
        i++;
    }

    return temp;
}

static uint32_t obj_str_hash(obj_str_t *obj_str) {
    return obj_bytearray_hash((const uint8_t *) obj_str->chars, obj_str->length);
}

static uint32_t obj_arr_hash(obj_arr_t *obj_arr) {
    return obj_bytearray_hash((const uint8_t *) obj_arr->buf, obj_arr->length);
}

static uint32_t obj_hash(obj_t *obj) {
    switch (obj->type) {
        case OBJ_TYPE_STRING:
            return obj_str_hash((obj_str_t *) obj);
        case OBJ_TYPE_BYTEARRAY:
            return obj_arr_hash((obj_arr_t *) obj);
        default:
            printf("No hash function for obj type %d\n", (uint32_t) obj->type);
            exit(1);
    }
}

uint32_t val_hash(val_t *val) {
    switch (val->type) {
        case VAL_TYPE_BYTE:
            return val->as.byteval;
        case VAL_TYPE_BOOL:
            return val->as.boolval;
        case VAL_TYPE_CHAR:
            return val->as.charval;
        case VAL_TYPE_INT:
            return (uint32_t) val->as.intval;
        case VAL_TYPE_UINT:
            return val->as.uintval;
        case VAL_TYPE_FLOAT:
            return val->as.floatval;
        case VAL_TYPE_OBJ:
            return obj_hash(AS_OBJ(val));
        default:
            printf("No hash function for val type %d\n", val->type);
            exit(1);
    }
}
