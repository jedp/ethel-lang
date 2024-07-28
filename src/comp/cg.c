#include <stdlib.h>
#include <stdio.h>

#include "../common/map.h"
#include "../common/ptr.h"
#include "../mem/mem.h"
#include "cg.h"
#include "comp.h"
#include "val.h"
#include "hash.h"

void cg_init(cg_t *cg) {
    cg->len = 0;
    cg->max = 0;
    cg->code_start = 0;
    cg->bytecode = NULL;
    cg->consts = map_new(MAP_NEW_BUCKETS, &val_hash, &val_eq);
}

void cg_free(cg_t *cg) {
    mem_free(cg->bytecode);
    mem_free(cg->consts);
    cg_init(cg);
}

void cg_byte(cg_t *cg, uint8_t byte) {
    if (cg->max < cg->len + 1) {
        cg->max = ARR_GROW_MAX_SIZE(cg->max);
        cg->bytecode = ARR_GROW(uint8_t, cg->bytecode, cg->max);

        if (cg->bytecode == NULL) {
            printf("Realloc failed!\n");
            exit(1);
        }
    }

    cg->bytecode[cg->len] = byte;
    cg->len++;
}

void cg_bytes(cg_t *cg, const uint8_t *bytes, size_t size) {
    for (size_t i = 0; i < size; i++) {
        cg_byte(cg, bytes[i]);
    }
}

uint32_t cg_header(cg_t *cg, const uint8_t *bytes, size_t size) {
    uint32_t offset = 0;

    // Magic
    if (!(bytes[offset++] == 'E' &&
          bytes[offset++] == 'T' &&
          bytes[offset++] == 'H' &&
          bytes[offset++] == 'L')) {
        return 0;
    }
    // Version
    if (!(bytes[offset++] == 0 && bytes[offset++] == 1)) {
        return 0;
    }

    uint8_t num_consts = bytes[offset++];
    if (num_consts == 0) {
        return offset;
    }

    for (uint8_t i = 1; i <= num_consts; i++) {
        const_type_t type = bytes[offset++];
        val_t v;
        uint8_t k;
        switch (type) {
            case CONST_INT: {
                // Ints are packed into as few bytes as possible,
                // least-significant byte first.
                // Extend sign bit if necessary.
                uint32_t uintval = 0;
                uint8_t intsize = bytes[offset++];
                uintval |= (bytes[offset++]) & 0xff;
                if (intsize > 1)
                    uintval |= (bytes[offset++] << 8) & 0xff00;
                if (intsize > 2)
                    uintval |= (bytes[offset++] << 16) & 0xff0000;
                if (intsize > 3)
                    uintval |= (bytes[offset++] << 24) & 0xff000000;
                if (intsize > 4)
                    return 0;
                // Extend sign bit.
                if (intsize < 4 && uintval & (1 << ((intsize - 1) * 8 + 7))) {
                    uintval |= (0xffffffff << intsize * 8);
                }
                v.type = VAL_TYPE_INT;
                v.as.intval = (int) uintval;
                cg_put_const(cg, v, &k);
                break;
            }
            case CONST_STRING: {
                // TODO enforce string max size?
                uint8_t strlen = bytes[offset++];
                v.type = VAL_TYPE_OBJ;
                v.as.objval = (obj_t *) obj_str_new((const char *) &bytes[offset], strlen);
                cg_put_const(cg, v, &k);
                offset += strlen;
                break;
            }
            case CONST_BYTEARRAY: {
                uint32_t arrlen = 0;
                // First four bytes are array length.
                arrlen |= (bytes[offset++]) & 0xff;
                arrlen |= (bytes[offset++] << 8) & 0xff00;
                arrlen |= (bytes[offset++] << 16) & 0xff0000;
                arrlen |= (bytes[offset++] << 24) & 0xff000000;
                v.type = VAL_TYPE_OBJ;
                v.as.objval = (obj_t*) obj_arr_new((const uint8_t*) &bytes[offset], arrlen);
                cg_put_const(cg, v, &k);
                offset += arrlen;
                break;
            }
            default:
                printf("We don't handle const type %d yet!\n", type);
                return 0;
        }
        if (k != i) {
            printf("ERROR: Created const index %d, but bytecode index is %d\n", k, i);
            return 0;
        }
    }

    return offset;
}

map_err_t cg_put_const(cg_t *cg, val_t v, uint8_t *k) {
    uint32_t next_k = cg->consts->buckets->nelems + 1;
    if (next_k > UINT8_MAX) {
        return MAP_TOO_MANY_ITEMS;
    }

    val_t *ek = (val_t *) mem_alloc(sizeof(val_t));
    ek->type = VAL_TYPE_UINT;
    ek->as.uintval = next_k;
    map_err_t err = map_put(cg->consts, ek, &v);

    *k = (uint8_t) next_k;

    return err;
}

map_err_t cg_get_const(cg_t *cg, uint8_t k, val_t *v) {
    val_t *ek = (val_t *) mem_alloc(sizeof(val_t));
    ek->type = VAL_TYPE_UINT;
    ek->as.uintval = k;
    val_t *found = map_get(cg->consts, ek);
    if (found == NULL) {
        return MAP_NOT_FOUND;
    }
    *v = *found;

    return MAP_OK;
}
