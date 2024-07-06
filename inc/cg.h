#pragma once

#include <inttypes.h>

#include "def.h"
#include "err.h"
#include "map.h"
#include "obj.h"

typedef struct cg_t {
    gc_header_t hdr;
    uint32_t len;
    uint32_t max;
    uint8_t *code;
    map_t *consts;
} cg_t;

void cg_init(cg_t *cg);

void cg_free(cg_t *cg);

void cg_byte(cg_t *cg, uint8_t byte);

void cg_bytes(cg_t *cg, bytearray_t *bytes);

error_t cg_add_const(cg_t *cg, uint32_t n, int v);

int cg_get_const(cg_t *cg, uint32_t n);
