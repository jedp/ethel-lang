#pragma once

#include <inttypes.h>

typedef struct cg_t {
    uint32_t len;
    uint32_t max;
    uint8_t *code;
} cg_t;

void cg_init(cg_t *cg);

void cg_free(cg_t *cg);

void cg_byte(cg_t *cg, uint8_t byte);
