#include "../inc/comp.h"
#include "../inc/str.h"

#define CODE_ARRAY_SIZE_INIT 256

void cg_new(cg_t *cg)
{
    // TODO: gc-able

    cg->code_len = 0;
    cg->code = bytearray_alloc(CODE_ARRAY_SIZE_INIT);
}

void cg_free(cg_t *cg)
{
    // Liberate me.
}
