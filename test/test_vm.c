#include "unity/unity.h"
#include "test_vm.h"
#include "../inc/cg.h"
#include "../inc/dict.h"
#include "../inc/op.h"
#include "../inc/str.h"
#include "../inc/vm.h"

void test_vm_init(void) {
    vm_t vm;
    vm_init(&vm);

    TEST_ASSERT_EQUAL(vm.code_size, vm.cg->len);
    TEST_ASSERT_EQUAL_PTR(vm.pc, vm.cg->code);
    TEST_ASSERT_NULL(vm.cg->code);

    vm_free(&vm);
}

void test_vm_load_code(void) {
    vm_t vm;
    vm_init(&vm);

    uint8_t bytes[] = {VM_OP_NOP, VM_OP_RET};
    bytearray_t *bytecode = bytearray_alloc_with_data(2, bytes);

    vm_load_code(&vm, bytecode);

    TEST_ASSERT_EQUAL(VM_OP_NOP, *vm.pc++);
    TEST_ASSERT_EQUAL(VM_OP_RET, *vm.pc);

    vm_free(&vm);
}

void test_vm_stack_int(void) {
    vm_t vm;
    vm_init(&vm);

    TEST_ASSERT_EQUAL_PTR(vm.stack->buf, vm.stack->top);

    vm_stack_push_int(&vm, 1234);
    vm_stack_push_int(&vm, 2345);
    vm_stack_push_int(&vm, 3456);

    TEST_ASSERT_EQUAL(3456, vm_stack_pop(&vm)->intval);
    TEST_ASSERT_EQUAL(2345, vm_stack_pop(&vm)->intval);
    TEST_ASSERT_EQUAL(1234, vm_stack_pop(&vm)->intval);
}

void test_vm_loadi(void) {
    vm_t vm;
    vm_init(&vm);

    error_t err;

    uint8_t bytes[] = {VM_OP_NOP, VM_OP_LOADI, 42, VM_OP_RET};
    bytearray_t *bytecode = bytearray_alloc_with_data(4, bytes);

    err = vm_load_code(&vm, bytecode);
    err |= cg_add_const(vm.cg, int_obj(42), int_obj(123));
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
    TEST_ASSERT_EQUAL(123, cg_get_const(vm.cg, int_obj(42))->intval);

    vm_interp(&vm);

    vm_free(&vm);
}

void test_vm() {
    RUN_TEST(test_vm_init);
    RUN_TEST(test_vm_load_code);
    RUN_TEST(test_vm_stack_int);
    RUN_TEST(test_vm_loadi);
}