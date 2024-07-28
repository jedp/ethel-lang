#pragma once

#include <stdint.h>
#include "obj.h"

typedef struct {
    obj_t hdr;
    uint32_t length;
    uint32_t buflen;
    uint8_t *buf;
} obj_arr_t;

obj_arr_t* obj_arr_new(const uint8_t* bytes, uint32_t length);
