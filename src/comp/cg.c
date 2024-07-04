#include <stdlib.h>

#include "arr.h"
#include "cg.h"

void cg_init(cg_t *cg) {
    cg->len = 0;
    cg->max = 0;
    cg->code = NULL;
}

void cg_free(cg_t *cg) {
    ARR_FREE(cg->code);
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