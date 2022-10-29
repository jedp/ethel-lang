#include <stdlib.h>
#include "../inc/mem.h"
#include "../inc/comp.h"
#include "../inc/str.h"

#define CODE_ARRAY_SIZE_INIT 256

void cg_new(cg_t *cg)
{
    cg->code_len = 0;
    cg->code = bytearray_alloc(CODE_ARRAY_SIZE_INIT);
}

void cg_byte(cg_t *cg, uint8_t b)
{
    // TODO resize

    bytearray_set(cg->code, b, cg->code_len);
    cg->code_len++;
}

void cg_free(cg_t *cg)
{
    // Liberate me.
}