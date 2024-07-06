#include <stdlib.h>

#include "arr.h"
#include "cg.h"
#include "def.h"
#include "map.h"

void cg_init(cg_t *cg) {
    cg->len = 0;
    cg->max = 0;
    cg->code = NULL;
    cg->consts = map_new(MAP_NEW_BUCKETS, &hash_primitive, &eq_primitive);
}

void cg_free(cg_t *cg) {
    mem_free(cg->code);
    mem_free(cg->consts);
    cg_init(cg);
}

void cg_byte(cg_t *cg, uint8_t byte) {
    if (cg->max < cg->len - 1) {
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

void cg_bytes(cg_t *cg, bytearray_t *bytes) {
    for (uint32_t i = 0; i < bytes->size; i++) {
        cg_byte(cg, bytes->data[i]);
    }
}

error_t cg_add_const(cg_t *cg, uint32_t n, int v) {
    map_elem_t *ek = (map_elem_t *) mem_alloc(sizeof(map_elem_t));
    ((gc_header_t *) ek)->type = VM_DATA_NO_GC;
    ek->type = MAP_ELEM_UINT_TYPE;
    ek->elem.uintval = n;
    map_elem_t *ev = (map_elem_t *) mem_alloc(sizeof(map_elem_t));
    ((gc_header_t *) ev)->type = VM_DATA_NO_GC;
    ev->type = MAP_ELEM_INT_TYPE;
    ev->elem.intval = v;
    return map_put(cg->consts, ek, ev);
}

int cg_get_const(cg_t *cg, uint32_t n) {
    map_elem_t *ek = (map_elem_t *) mem_alloc(sizeof(map_elem_t));
    ((gc_header_t *) ek)->type = VM_DATA_NO_GC;
    ek->type = MAP_ELEM_UINT_TYPE;
    ek->elem.uintval = n;
    map_elem_t *found = map_get(cg->consts, ek);
    if (found == NULL) {
        // TODO here's a weird number to signal problems. (decimal 90.)
        return 0x5a;
    }
    return found->elem.intval;
}
