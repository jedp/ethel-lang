#pragma once

#include "def.h"
#include "err.h"

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
    VAL_TYPE_STRING_PTR,
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
        const char *stringval_ptr;
        size_t addrval;
    } as;
} val_t;
