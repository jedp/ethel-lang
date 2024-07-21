#include <stdlib.h>

#include "../common/def.h"
#include "../common/map.h"
#include "cmem.h"
#include "cg.h"

void cg_init(cg_t *cg) {
    cg->len = 0;
    cg->max = 0;
    cg->code = NULL;
    cg->consts = map_new(MAP_NEW_BUCKETS, &hash_primitive, &eq_primitive);
}

void cg_free(cg_t *cg) {
    comp_free(cg->code);
    comp_free(cg->consts);
    cg_init(cg);
}

void cg_byte(cg_t *cg, uint8_t byte) {
    if (cg->max < cg->len + 1) {
        cg->max = ARR_GROW_MAX_SIZE(cg->max);
        cg->code = ARR_GROW(uint8_t, cg->code, cg->max);

        if (cg->code == NULL) {
            printf("Realloc failed!\n");
            exit(1);
        }
    }

    cg->code[cg->len] = byte;
    cg->len++;
}

void cg_bytes(cg_t *cg, uint8_t *bytes, size_t size) {
    for (size_t i = 0; i < size; i++) {
        cg_byte(cg, bytes[i]);
    }
}

map_err_t cg_put_const(cg_t *cg, map_elem_t v, uint8_t *k) {
    uint32_t next_k = cg->consts->buckets->nelems + 1;
    if (next_k > UINT8_MAX) {
        return MAP_TOO_MANY_ITEMS;
    }

    map_elem_t *ek = (map_elem_t *) comp_alloc(sizeof(map_elem_t));
    ek->type = MAP_ELEM_UINT_TYPE;
    ek->elem.uintval = next_k;
    map_err_t err = map_put(cg->consts, ek, &v);

    *k = (uint8_t) next_k;

    return err;
}

map_err_t cg_get_const(cg_t *cg, uint8_t k, map_elem_t *v) {
    map_elem_t *ek = (map_elem_t *) comp_alloc(sizeof(map_elem_t));
    ek->type = MAP_ELEM_UINT_TYPE;
    ek->elem.uintval = k;
    map_elem_t *found = map_get(cg->consts, ek);
    if (found == NULL) {
        return MAP_NOT_FOUND;
    }
    *v = *found;

    return MAP_OK;
}
