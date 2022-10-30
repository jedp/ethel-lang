#include <stdlib.h>
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/vm.h"
#include "../inc/comp.h"

#define CONST_POOL_SIZE INT8_MAX

static obj_arr_t *obj_arr_alloc(uint32_t elems)
{
    // TODO: gc-able
    obj_arr_t *a = mem_alloc(sizeof(obj_arr_t));
    a->arr_max = elems;
    a->arr_len = 0;
    a->values = mem_alloc(elems * sizeof(obj_t));

    return a;
}

static void obj_arr_append(obj_arr_t *a, obj_t *obj)
{
    // TODO: resize
    if (a->arr_max <= a->arr_len) exit(1);

    a->values[a->arr_len] = obj;
    a->arr_len++;
}

static int obj_arr_index_of(obj_arr_t *a, obj_t *obj)
{
    return -1;
}

void consts_new(obj_arr_t *consts)
{
    consts->arr_len = 0;
    consts->arr_max = CONST_POOL_SIZE;
    consts->values = mem_alloc(sizeof(obj_t) * CONST_POOL_SIZE);
}

void vm_reset(vm_t *vm)
{
    vm->env = new_env();

    cg_t *cg;
    cg = mem_alloc(sizeof(cg_t));

    cg_new(cg);

    obj_arr_t *consts;
    consts = mem_alloc(sizeof(obj_arr_t));

    consts_new(consts);

    vm->cg = cg;
    vm->consts = consts;
    printf("vm %p\n", vm);
    printf("cg %p, cg->code %p\n", vm->cg, vm->cg->code);
    printf("consts %p\n", vm->consts);
}

void cg_byte(vm_t *vm, uint8_t b)
{
    printf("cg byte %x\n", b);
    // TODO resize
    bytearray_set(vm->cg->code, b, vm->cg->code_len);
    vm->cg->code_len++;
}

uint32_t add_const(vm_t *vm, obj_t *c)
{
    obj_arr_append(vm->consts, c);
    return vm->consts->arr_len;
}
