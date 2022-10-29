#ifndef __VM_H
#define __VM_H

#include <inttypes.h>
#include "def.h"

typedef enum {
    OP_NO_OP,
    OP_RET,
} op_t;

static const char *op_names[] = {
        "ERROR_INVALID",
        "RETURN",
};

typedef struct {
    uint32_t code_len;
    bytearray_t *code;
} cg_t;

#endif
