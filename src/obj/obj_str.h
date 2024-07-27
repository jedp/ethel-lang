#pragma once

#include <stdint.h>
#include "obj.h"

typedef struct {
    obj_t hdr;
    uint32_t length;
    char *chars;
} obj_str_t;

obj_str_t* obj_str_new(const char* chars, uint32_t length);

char *obj_str_to_c(obj_str_t *obj_str);
