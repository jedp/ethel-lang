#pragma once

#include <stdint.h>

typedef enum {
    OBJ_TYPE_ERROR_NONE = 0,
    OBJ_TYPE_STRING,
} obj_type_t;

typedef struct {
#ifdef BUILD64
    uint64_t type: 52;
#else
    unsigned int type : 20;
#endif
    unsigned int flags: 8;
    unsigned int children: 4;
} obj_t;

obj_t *obj_new(uint32_t size, obj_type_t type);
