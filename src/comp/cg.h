#pragma once

#include <inttypes.h>

#include "../common/def.h"
#include "../common/err.h"
#include "../common/map.h"

typedef struct cg_t {
    uint32_t len;
    uint32_t max;
    uint8_t *code;
    map_t *consts;
} cg_t;

void cg_init(cg_t *cg);

void cg_free(cg_t *cg);

void cg_byte(cg_t *cg, uint8_t byte);

void cg_bytes(cg_t *cg, uint8_t *bytes, size_t size);

/*
 * Put a const v in the constant pool, writing back its index.
 *
 * The indexes are generated sequentially.
 */
map_err_t cg_put_const(cg_t *cg, map_elem_t v, uint8_t *k);

/*
 * Get a const by its index, writing back its value.
 */
map_err_t cg_get_const(cg_t *cg, uint8_t k, map_elem_t *v);
