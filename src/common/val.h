#pragma once

#include <stdbool.h>
#include "../obj/obj.h"
#include "../obj/obj_str.h"
#include "err.h"

#define AS_OBJ(val) ((obj_t*) (val)->as.objval)
#define AS_OBJ_STR(val) ((obj_str_t*) AS_OBJ(val))


typedef enum {
    MAP_OK,
    MAP_NOT_FOUND,
    MAP_TOO_MANY_ITEMS,
} map_err_t;

typedef enum {
    VAL_TYPE_ERR_NONE,
    VAL_TYPE_BYTE,
    VAL_TYPE_BOOL,
    VAL_TYPE_CHAR,
    VAL_TYPE_INT,
    VAL_TYPE_UINT,
    VAL_TYPE_FLOAT,
    VAL_TYPE_OBJ,
    VAL_TYPE_ADDR,
} map_elem_type_t;

typedef struct {
    map_elem_type_t type;
    union {
        uint8_t byteval;
        uint8_t boolval;
        unsigned char charval;
        int intval;
        uint32_t uintval;
        float floatval;
        obj_t *objval;
        size_t addrval;
    } as;
} val_t;

bool val_eq(val_t *a, val_t *b);
