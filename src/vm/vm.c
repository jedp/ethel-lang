#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include "../mem/mem.h"
#include "vm.h"
#include "hash.h"

void runtime_error(vm_t *vm, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fflush(stdout);
    fflush(stderr);

    fprintf(stderr, "Runtime error: ");
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);

    fflush(stderr);
    va_end(args);

    vm_stack_reset(vm);
}

bool runtime_check(vm_t *vm, bool condition, const char *message) {
    if (condition)
        return true;

    runtime_error(vm, message);

    return false;
}

vm_err_t vm_stack_reset(vm_t *vm) {
    vm->stack->top = vm->stack->buf;
    return VM_ERR_NO_ERROR;
}

map_err_t vm_map_put(map_t *map, val_t v, uint8_t *k) {
    uint32_t next_k = map->buckets->nelems + 1;
    if (next_k > UINT8_MAX) {
        return MAP_TOO_MANY_ITEMS;
    }

    val_t *ek = (val_t *) mem_alloc(sizeof(val_t));
    ek->type = VAL_TYPE_UINT;
    ek->as.uintval = next_k;
    map_err_t err = map_put(map, ek, &v);

    *k = (uint8_t) next_k;

    return err;
}

map_err_t vm_map_get(map_t *map, uint8_t k, val_t *v) {
    val_t *ek = (val_t *) mem_alloc(sizeof(val_t));
    ek->type = VAL_TYPE_UINT;
    ek->as.uintval = k;
    val_t *found = map_get(map, ek);
    if (found == NULL) {
        return MAP_NOT_FOUND;
    }
    *v = *found;

    return MAP_OK;
}

vm_err_t vm_init(vm_t *vm, const uint8_t *bytecode, uint32_t length) {
    // A stack of objects. Top is a pointer into buf.
    // Top always points to the next value to be filled.
    // If top == buf, stack is empty.
    vm_stack_t *stack = mem_alloc(sizeof(vm_stack_t));
    if (stack == NULL) {
        return VM_ERR_OUT_OF_MEMORY;
    }

    stack->size = 0;
    stack->top = stack->buf;

    vm->bytecode = (uint8_t *) bytecode;
    vm->pc = vm->bytecode;
    vm->code_start = 0;
    vm->bytecode_size = length;
    vm->stack = stack;

    vm->consts = map_new(MAP_NEW_BUCKETS, &val_hash, &val_eq);

    return VM_ERR_NO_ERROR;
}

vm_err_t vm_free(vm_t *vm) {
    return vm_init(vm, NULL, 0);
}
