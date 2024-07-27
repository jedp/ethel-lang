#pragma once

#include <inttypes.h>

#include "../common/def.h"
#include "err.h"
#include "../common/map.h"
#include "val.h"

typedef struct cg_t {
    uint32_t len;
    uint32_t max;
    uint32_t code_start;
    uint8_t *bytecode;
    map_t *consts;
} cg_t;

void cg_init(cg_t *cg);

void cg_free(cg_t *cg);

void cg_byte(cg_t *cg, uint8_t byte);

void cg_bytes(cg_t *cg, const uint8_t *bytes, size_t size);

/*
 * Parse the header and return the offset of the start of code.
 */
uint32_t cg_header(cg_t *cg, const uint8_t *bytes, size_t size);

/*
 * Put a const v in the constant pool, writing back its index.
 *
 * The indexes are generated sequentially.
 */
map_err_t cg_put_const(cg_t *cg, val_t v, uint8_t *k);

/*
 * Get a const by its index, writing back its value.
 */
map_err_t cg_get_const(cg_t *cg, uint8_t k, val_t *v);
