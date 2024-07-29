#pragma once

#include <stddef.h>
#include "vm.h"

vm_stack_elem_t vm_stack_elem_new(void);

vm_err_t vm_stack_push(vm_t *vm, vm_stack_elem_t *e);

vm_err_t vm_stack_push_byte(vm_t *vm, uint8_t b);

vm_err_t vm_stack_push_byte_as_int32(vm_t *vm, uint8_t b);

vm_err_t vm_stack_push_int32(vm_t *vm, int i);

vm_err_t vm_stack_push_boolean(vm_t *vm, bool z);

vm_err_t vm_stack_push_obj(vm_t *vm, obj_t *obj);

vm_err_t vm_stack_push_nil(vm_t *vm);

vm_stack_elem_t *vm_stack_peek(vm_t *vm);

vm_stack_elem_t *vm_stack_pop(vm_t *vm);

uint8_t vm_stack_size(vm_t *vm);

void vm_print_val(const vm_stack_elem_t *elem, char *string, uint8_t max_length);
