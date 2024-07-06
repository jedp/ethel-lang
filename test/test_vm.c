#include "unity/unity.h"
#include "test_vm.h"
#include "../src/comp/cg.h"
#include "../src/comp/map.h"
#include "../src/comp/op.h"
#include "../src/comp/vm.h"

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

    vm_load_code(&vm, bytes, sizeof(bytes));

    TEST_ASSERT_EQUAL(VM_OP_NOP, *vm.pc++);
    TEST_ASSERT_EQUAL(VM_OP_RET, *vm.pc);

    vm_free(&vm);
}

void test_vm_stack_push_int(void) {
    vm_t vm;
    vm_init(&vm);

    TEST_ASSERT_EQUAL_PTR(vm.stack->buf, vm.stack->top);

    vm_stack_push_int(&vm, 1234);
    vm_stack_push_int(&vm, 2345);
    vm_stack_push_int(&vm, 3456);

    TEST_ASSERT_EQUAL(3456, vm_stack_pop(&vm)->intval);
    TEST_ASSERT_EQUAL(2345, vm_stack_pop(&vm)->intval);
    TEST_ASSERT_EQUAL(1234, vm_stack_pop(&vm)->intval);

    vm_free(&vm);
}

void test_vm_stack_push(void) {
    vm_t vm;
    vm_init(&vm);

    vm_stack_elem_t i = {.type = VM_STACK_INT_TYPE, .intval = 42};
    vm_stack_elem_t f = {.type = VM_STACK_FLOAT_TYPE, .floatval = 2.34};
    vm_stack_elem_t b = {.type = VM_STACK_BYTE_TYPE, .byteval = 0xff};
    vm_stack_elem_t y = {.type = VM_STACK_BOOL_TYPE, .boolval = 1};

    vm_stack_push(&vm, &y);
    vm_stack_push(&vm, &b);
    vm_stack_push(&vm, &f);
    vm_stack_push(&vm, &i);

    TEST_ASSERT_EQUAL(42, vm_stack_pop(&vm)->intval);
    TEST_ASSERT_EQUAL(2.34, vm_stack_pop(&vm)->floatval);
    TEST_ASSERT_EQUAL(0xff, vm_stack_pop(&vm)->byteval);
    TEST_ASSERT_EQUAL(1, vm_stack_pop(&vm)->boolval);

    vm_free(&vm);
}

void test_vm_loadi(void) {
    vm_t vm;
    vm_init(&vm);

    error_t err = ERR_NO_ERROR;

    uint8_t bytes[] = {VM_OP_NOP, VM_OP_LOADI, 42, VM_OP_RET};

    err |= vm_load_code(&vm, bytes, sizeof(bytes));

    // Hand-code the interpretation part.
    err |= cg_add_const(vm.cg, 42, 123);

    err |= vm_interp(&vm);
    TEST_ASSERT_EQUAL(ERR_VM_INTERP_OK, err);
    TEST_ASSERT_EQUAL(123, vm_stack_peek(&vm)->intval);

    vm_free(&vm);
}

void test_vm_stack_negate(void) {
    vm_t vm;
    vm_init(&vm);

    error_t err = ERR_NO_ERROR;

    uint8_t bytes[] = {
        VM_OP_LOADI, 42,
        VM_OP_NEGATE,
        VM_OP_RET
    };

    err |= vm_load_code(&vm, bytes, sizeof(bytes));

    // Hand-code the interpretation part.
    err |= cg_add_const(vm.cg, 42, 123);

    err |= vm_interp(&vm);
    TEST_ASSERT_EQUAL(ERR_VM_INTERP_OK, err);
    TEST_ASSERT_EQUAL(-123, vm_stack_peek(&vm)->intval);

    vm_free(&vm);
}

void test_vm_stack_add(void) {
    vm_t vm;
    vm_init(&vm);

    error_t err = ERR_NO_ERROR;

    // 123 + 456
    uint8_t bytes[] = {
        VM_OP_LOADI, 1,
        VM_OP_LOADI, 2,
        VM_OP_ADD,
        VM_OP_RET
    };

    err |= vm_load_code(&vm, bytes, sizeof(bytes));

    // Hand-code the interpretation part.
    err |= cg_add_const(vm.cg, 1, 123);
    err |= cg_add_const(vm.cg, 2, 456);

    err |= vm_interp(&vm);
    TEST_ASSERT_EQUAL(ERR_VM_INTERP_OK, err);
    TEST_ASSERT_EQUAL(579, vm_stack_peek(&vm)->intval);

    vm_free(&vm);
}

void test_vm_stack_sub(void) {
    vm_t vm;
    vm_init(&vm);

    error_t err = ERR_NO_ERROR;

    // 5 - 2
    uint8_t bytes[] = {
        VM_OP_LOADI, 1,
        VM_OP_LOADI, 2,
        VM_OP_SUB,
        VM_OP_RET
    };

    err |= vm_load_code(&vm, bytes, sizeof(bytes));

    // Hand-code the interpretation part.
    err |= cg_add_const(vm.cg, 1, 5);
    err |= cg_add_const(vm.cg, 2, 2);

    err |= vm_interp(&vm);
    TEST_ASSERT_EQUAL(ERR_VM_INTERP_OK, err);
    TEST_ASSERT_EQUAL(3, vm_stack_peek(&vm)->intval);

    vm_free(&vm);
}

void test_vm_stack_mul(void) {
    vm_t vm;
    vm_init(&vm);

    error_t err = ERR_NO_ERROR;

    // 5 * 3
    uint8_t bytes[] = {
        VM_OP_LOADI, 1,
        VM_OP_LOADI, 2,
        VM_OP_MUL,
        VM_OP_RET
    };

    err |= vm_load_code(&vm, bytes, sizeof(bytes));

    // Hand-code the interpretation part.
    err |= cg_add_const(vm.cg, 1, 5);
    err |= cg_add_const(vm.cg, 2, 3);

    err |= vm_interp(&vm);
    TEST_ASSERT_EQUAL(ERR_VM_INTERP_OK, err);
    TEST_ASSERT_EQUAL(15, vm_stack_peek(&vm)->intval);

    vm_free(&vm);
}

void test_vm_stack_div(void) {
    vm_t vm;
    vm_init(&vm);

    error_t err = ERR_NO_ERROR;

    // Integer division. 12 / 2
    uint8_t bytes[] = {
        VM_OP_LOADI, 1,
        VM_OP_LOADI, 2,
        VM_OP_DIV,
        VM_OP_RET
    };

    err |= vm_load_code(&vm, bytes, sizeof(bytes));

    // Hand-code the interpretation part.
    err |= cg_add_const(vm.cg, 1, 12);
    err |= cg_add_const(vm.cg, 2, 2);

    err |= vm_interp(&vm);
    TEST_ASSERT_EQUAL(ERR_VM_INTERP_OK, err);
    TEST_ASSERT_EQUAL(6, vm_stack_peek(&vm)->intval);

    vm_free(&vm);
}

void test_vm_stack_rem(void) {
    vm_t vm;
    vm_init(&vm);

    error_t err = ERR_NO_ERROR;

    // Modulus. 11 % 3
    uint8_t bytes[] = {
        VM_OP_LOADI, 1,
        VM_OP_LOADI, 2,
        VM_OP_REM,
        VM_OP_RET
    };

    err |= vm_load_code(&vm, bytes, sizeof(bytes));

    // Hand-code the interpretation part.
    err |= cg_add_const(vm.cg, 1, 11);
    err |= cg_add_const(vm.cg, 2, 3);

    err |= vm_interp(&vm);
    TEST_ASSERT_EQUAL(ERR_VM_INTERP_OK, err);
    TEST_ASSERT_EQUAL(2, vm_stack_peek(&vm)->intval);

    vm_free(&vm);
}


void test_vm_stack_arith(void) {
    vm_t vm;
    vm_init(&vm);

    error_t err = ERR_NO_ERROR;

    // (2 - 1) * (3 + 5) / 2 % 3 = 1
    uint8_t bytes[] = {
        VM_OP_LOADI, 1,
        VM_OP_LOADI, 2,
        VM_OP_SUB,
        VM_OP_LOADI, 3,
        VM_OP_LOADI, 4,
        VM_OP_ADD,
        VM_OP_MUL,
        VM_OP_LOADI, 1,
        VM_OP_DIV,
        VM_OP_LOADI, 3,
        VM_OP_REM,
        VM_OP_RET
    };

    err |= vm_load_code(&vm, bytes, sizeof(bytes));

    // Hand-code the interpretation part.
    err |= cg_add_const(vm.cg, 1, 2);
    err |= cg_add_const(vm.cg, 2, 1);
    err |= cg_add_const(vm.cg, 3, 3);
    err |= cg_add_const(vm.cg, 4, 5);

    err |= vm_interp(&vm);
    TEST_ASSERT_EQUAL(ERR_VM_INTERP_OK, err);
    TEST_ASSERT_EQUAL(1, vm_stack_peek(&vm)->intval);

    vm_free(&vm);
}

void test_vm_stack_load_imm(void) {
    vm_t vm;
    vm_init(&vm);

    error_t err = ERR_NO_ERROR;

    uint8_t bytes[] = {
        VM_OP_LOADI, 1,
        VM_OP_LOADI_1N,
        VM_OP_LOADI_1,
        VM_OP_ADD,
        VM_OP_LOADI_0,
        VM_OP_ADD,
        VM_OP_ADD,
        VM_OP_RET
    };

    err |= vm_load_code(&vm, bytes, sizeof(bytes));

    // Hand-code the interpretation part.
    err |= cg_add_const(vm.cg, 1, 42);

    err |= vm_interp(&vm);
    TEST_ASSERT_EQUAL(ERR_VM_INTERP_OK, err);
    TEST_ASSERT_EQUAL(42, vm_stack_peek(&vm)->intval);

    vm_free(&vm);
}

void test_vm_stack_inc_dec(void) {
    vm_t vm;
    vm_init(&vm);

    error_t err = ERR_NO_ERROR;

    uint8_t bytes[] = {
        VM_OP_LOADI_0,
        VM_OP_INC,
        VM_OP_INC,
        VM_OP_DEC,
        VM_OP_RET,
    };

    err |= vm_load_code(&vm, bytes, sizeof(bytes));

    err |= vm_interp(&vm);
    TEST_ASSERT_EQUAL(ERR_VM_INTERP_OK, err);
    TEST_ASSERT_EQUAL(1, vm_stack_peek(&vm)->intval);

    vm_free(&vm);
}

void test_vm() {
    RUN_TEST(test_vm_init);
    RUN_TEST(test_vm_load_code);
    RUN_TEST(test_vm_stack_push_int);
    RUN_TEST(test_vm_stack_push);
    RUN_TEST(test_vm_loadi);
    RUN_TEST(test_vm_stack_negate);
    RUN_TEST(test_vm_stack_add);
    RUN_TEST(test_vm_stack_sub);
    RUN_TEST(test_vm_stack_mul);
    RUN_TEST(test_vm_stack_div);
    RUN_TEST(test_vm_stack_rem);
    RUN_TEST(test_vm_stack_arith);
    RUN_TEST(test_vm_stack_load_imm);
    RUN_TEST(test_vm_stack_inc_dec);
}