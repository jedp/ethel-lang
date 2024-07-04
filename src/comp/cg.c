#include <stdlib.h>

#include "arr.h"
#include "cg.h"
#include "dict.h"

void cg_init(cg_t *cg) {
    cg->len = 0;
    cg->max = 0;
    cg->code = NULL;
    cg->consts = dict_obj();
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

        // Realloc failed?
        if (cg->code == NULL) {
            exit(1);
        }
    }

    cg->code[cg->len] = byte;
    cg->len++;
}

error_t cg_add_const(cg_t *cg, obj_t *k, obj_t *v) {
    return dict_put(cg->consts, k, v);
}

obj_t* cg_get_const(cg_t *cg, obj_t *k) {
    return dict_get(cg->consts, k);
}
