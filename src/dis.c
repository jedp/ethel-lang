#include <stdlib.h>
#include <stdio.h>
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/dis.h"

static uint32_t dis_op(cg_t *cg, uint32_t offset)
{
    printf("%04d ", offset);

    uint8_t op = bytearray_get(cg->code, offset);
    switch(op) {
        case OP_RET:
            printf("%s\n", op_names[op]);
            return offset + 1;
        default:
            printf("Unknown opcode: %d\n", op);
            exit(1);
    }
}

void dis(vm_t *vm, const char *name)
{
    printf("Disassembly of %s:\n", name);

    for (uint32_t i = 0; i < vm->cg->code_len;) {
        i = dis_op(vm->cg, i);
    }
}

int main(int argc, const char **argv)
{
    (void) argc;
    (void) argv;

    mem_init('Q');

    vm_t *vm;
    vm = mem_alloc(sizeof(vm_t));

    vm_reset(vm);

    // Hello, world.
    cg_byte(vm, OP_RET);

    dis(vm, "Hello, world!");
    return 0;
}