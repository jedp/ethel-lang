#include <stdlib.h>
#include <stdio.h>
#include "../inc/mem.h"
#include "../inc/comp.h"
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

void dis(cg_t *cg, const char *name)
{
    printf("Disassembly of %s:\n", name);

    for (uint32_t i = 0; i < cg->code_len;) {
        i = dis_op(cg, i);
    }
}

int main(int argc, const char **argv)
{
    (void) argc;
    (void) argv;

    // TODO - the heap is global
    mem_init(0);

    // Hello, world.
    cg_t cg;
    cg_new(&cg);
    cg_byte(&cg, OP_RET);

    dis(&cg, "Hello, world!");
    return 0;
}