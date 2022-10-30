#ifndef __VM_H
#define __VM_H

#include <inttypes.h>
#include "env.h"
#include "def.h"
#include "obj.h"

typedef enum {
    OP_NO_OP,
    OP_RET,
    OP_CONST,
} op_t;

static const char *op_names[] = {
        "ERROR_INVALID",
        "RETURN",
        "PUSH_CONSTANT",
};

typedef struct {
    uint8_t arr_len;
    uint8_t arr_max;
    obj_t **values;
} obj_arr_t;

typedef struct {
    uint32_t code_len;
    bytearray_t *code;
} cg_t;

typedef struct {
    env_t *env;
    cg_t *cg;
    obj_arr_t *consts;
} vm_t;

void vm_reset(vm_t *vm);

/* Code gen one byte. */
void cg_byte(vm_t *vm, uint8_t b);

/* Add a reference to object c to the constant pool. */
uint32_t add_const(vm_t *vm, obj_t *c);

#endif
