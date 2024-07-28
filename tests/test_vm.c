#include "unity/unity.h"
#include "test_vm.h"
#include "../src/comp/cg.h"
#include "../src/comp/comp.h"
#include "../src/common/op.h"
#include "../src/vm/vm.h"

#define EMPTY_CONST_POOL (0)

typedef struct {
    obj_type_t type;
    uint32_t length;
} obj_details_t;

static error_t vm_init_and_exec(vm_t *vm, uint8_t *bytes, uint16_t size) {
    vm_init(vm);
    error_t err = ERR_NO_ERROR;

    err |= vm_load_code(vm, bytes, size);
    err |= vm_exec(vm);

    return err;
}

static void test_program_object(uint8_t *bytes, uint16_t size,
                                obj_type_t expect_obj_type,
                                error_t expect_err) {
    vm_t vm;
    error_t err = vm_init_and_exec(&vm, bytes, size);

    TEST_ASSERT_EQUAL(expect_err, err);

    if (expect_obj_type != 0) {
        TEST_ASSERT_EQUAL(expect_obj_type, vm_stack_peek(&vm)->as.objval->type);
    }

    vm_free(&vm);
}

static void test_program_primitive(uint8_t *bytes, uint16_t size,
                                   vm_stack_elem_t expect_stack_top_primitive,
                                   error_t expect_err) {
    vm_t vm;
    error_t err = vm_init_and_exec(&vm, bytes, size);

    TEST_ASSERT_EQUAL(expect_err, err);

    if (expect_stack_top_primitive.type != 0) {
        TEST_ASSERT_EQUAL(expect_stack_top_primitive.type, vm_stack_peek(&vm)->type);
        switch (expect_stack_top_primitive.type) {
            case VM_STACK_BYTE_TYPE:
                TEST_ASSERT_EQUAL(expect_stack_top_primitive.as.byteval, vm_stack_peek(&vm)->as.byteval);
                break;
            case VM_STACK_INT_TYPE:
                TEST_ASSERT_EQUAL(expect_stack_top_primitive.as.intval, vm_stack_peek(&vm)->as.intval);
                break;
            default:
                printf("Tests don't handle primitive type %d yet\n", expect_stack_top_primitive.type);
                TEST_ABORT();
        }
    }

    vm_free(&vm);
}

void test_vm_init(void) {
    vm_t vm;
    vm_init(&vm);

    TEST_ASSERT_EQUAL(vm.bytecode_size, vm.cg->len);
    TEST_ASSERT_EQUAL_PTR(vm.pc, vm.cg->bytecode);
    TEST_ASSERT_NULL(vm.cg->bytecode);

    vm_free(&vm);
}

void test_vm_load_code(void) {
    vm_t vm;
    vm_init(&vm);

    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        EMPTY_CONST_POOL,
        VM_OP_NOP,
        VM_OP_RET};

    vm_load_code(&vm, bytes, sizeof(bytes));

    TEST_ASSERT_EQUAL(VM_OP_NOP, *vm.pc++);
    TEST_ASSERT_EQUAL(VM_OP_RET, *vm.pc);

    vm_free(&vm);
}

void test_vm_stack_push_byte(void) {
    vm_t vm;
    vm_init(&vm);

    TEST_ASSERT_EQUAL_PTR(vm.stack->buf, vm.stack->top);

    vm_stack_push_byte(&vm, 0x88);
    vm_stack_push_byte(&vm, 0x00);
    vm_stack_push_byte(&vm, 0xff);

    TEST_ASSERT_EQUAL(0xff, vm_stack_pop(&vm)->as.byteval);
    TEST_ASSERT_EQUAL(0x00, vm_stack_pop(&vm)->as.byteval);
    TEST_ASSERT_EQUAL(0x88, vm_stack_pop(&vm)->as.byteval);

    vm_free(&vm);
}

void test_vm_stack_push_int(void) {
    vm_t vm;
    vm_init(&vm);

    TEST_ASSERT_EQUAL_PTR(vm.stack->buf, vm.stack->top);

    vm_stack_push_int32(&vm, 1234);
    vm_stack_push_int32(&vm, 2345);
    vm_stack_push_int32(&vm, 3456);

    TEST_ASSERT_EQUAL(3456, vm_stack_pop(&vm)->as.intval);
    TEST_ASSERT_EQUAL(2345, vm_stack_pop(&vm)->as.intval);
    TEST_ASSERT_EQUAL(1234, vm_stack_pop(&vm)->as.intval);

    vm_free(&vm);
}

void test_vm_stack_push_byte_as_int32(void) {
    vm_t vm;
    vm_init(&vm);

    TEST_ASSERT_EQUAL_PTR(vm.stack->buf, vm.stack->top);

    vm_stack_push_byte_as_int32(&vm, 0xd6);
    TEST_ASSERT_EQUAL(-42, vm_stack_pop(&vm)->as.intval);

    vm_free(&vm);
}

void test_vm_stack_push(void) {
    vm_t vm;
    vm_init(&vm);

    vm_stack_elem_t i = {.type = VM_STACK_INT_TYPE, .as.intval = 42};
    vm_stack_elem_t f = {.type = VM_STACK_FLOAT_TYPE, .as.floatval = 2.34f};
    vm_stack_elem_t b = {.type = VM_STACK_BYTE_TYPE, .as.byteval = 0xff};
    vm_stack_elem_t y = {.type = VM_STACK_BOOL_TYPE, .as.boolval = 1};

    vm_stack_push(&vm, &y);
    vm_stack_push(&vm, &b);
    vm_stack_push(&vm, &f);
    vm_stack_push(&vm, &i);

    TEST_ASSERT_EQUAL(42, vm_stack_pop(&vm)->as.intval);
    TEST_ASSERT_EQUAL(2.34, vm_stack_pop(&vm)->as.floatval);
    TEST_ASSERT_EQUAL(0xff, vm_stack_pop(&vm)->as.byteval);
    TEST_ASSERT_EQUAL(1, vm_stack_pop(&vm)->as.boolval);

    vm_free(&vm);
}

void test_vm_iconst(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        // Const pool
        1, CONST_INT, 1, 42,
        // Code
        VM_OP_NOP,
        VM_OP_ICONST, 1,
        VM_OP_RET};

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 42};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_iconst_negative(void) {
    // -32768
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        // Const pool
        1, CONST_INT, 2, 0x00, 0x80,
        // Code
        VM_OP_NOP,
        VM_OP_ICONST, 1,
        VM_OP_RET};

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= -32768};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_sconst(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        // Const pool
        1, CONST_STRING, 5, 'E', 't', 'h', 'e', 'l',
        // Code
        VM_OP_SCONST, 1,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= 0};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_stack_negate(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        // Const pool
        1, CONST_INT, 1, 123,
        // Code
        VM_OP_ICONST, 1,
        VM_OP_NEG,
        VM_OP_RET
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= -123};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_stack_add(void) {
    // 123 + 456
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        // Const pool
        2,
        CONST_INT, 1, 123,
        CONST_INT, 2, 200, 1, // 456
        // Code
        VM_OP_ICONST, 1,
        VM_OP_ICONST, 2,
        VM_OP_ADD,
        VM_OP_RET
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 579};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_stack_sub(void) {
    // 5 - 2
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        // Const pool
        2,
        CONST_INT, 1, 5,
        CONST_INT, 1, 2,
        // Code
        VM_OP_ICONST, 1,
        VM_OP_ICONST, 2,
        VM_OP_SUB,
        VM_OP_RET
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 3};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_stack_mul(void) {
    // 5 * 3
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        // Const pool
        2,
        CONST_INT, 1, 5,
        CONST_INT, 1, 3,
        // Code
        VM_OP_ICONST, 1,
        VM_OP_ICONST, 2,
        VM_OP_MUL,
        VM_OP_RET
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 15};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_stack_div(void) {
    // Integer division. 12 / 2
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        // Const pool
        2,
        CONST_INT, 1, 12,
        CONST_INT, 1, 2,
        // Code
        VM_OP_ICONST, 1,
        VM_OP_ICONST, 2,
        VM_OP_DIV,
        VM_OP_RET
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 6};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_stack_rem(void) {
    // Modulus. 11 % 3
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        // Const pool
        2,
        CONST_INT, 1, 11,
        CONST_INT, 1, 3,
        // Code
        VM_OP_ICONST, 1,
        VM_OP_ICONST, 2,
        VM_OP_REM,
        VM_OP_RET
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 2};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_stack_arith(void) {
    // (2 - 1) * (3 + 5) / 2 % 3 = 1
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        // Const pool
        4,
        CONST_INT, 1, 2,
        CONST_INT, 1, 1,
        CONST_INT, 1, 3,
        CONST_INT, 1, 5,
        // Code
        VM_OP_ICONST, 1,
        VM_OP_ICONST, 2,
        VM_OP_SUB,
        VM_OP_ICONST, 3,
        VM_OP_ICONST, 4,
        VM_OP_ADD,
        VM_OP_MUL,
        VM_OP_ICONST, 1,
        VM_OP_DIV,
        VM_OP_ICONST, 3,
        VM_OP_REM,
        VM_OP_RET
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 1};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_booleans(void) {
    // true or false and not true
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        EMPTY_CONST_POOL,
        VM_OP_ZPUSH_T,
        VM_OP_ZPUSH_F,
        VM_OP_ZPUSH_T,
        VM_OP_LOGICAL_NOT,
        VM_OP_LOGICAL_AND,
        VM_OP_LOGICAL_OR,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_BOOL_TYPE, .as.intval= 1};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_non_boolean_booleans(void) {
    // 42 or -1 and not nil
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        1,
        CONST_INT, 1, 42,
        VM_OP_ICONST, 1,
        VM_OP_IPUSH_1N,
        VM_OP_NIL,
        VM_OP_LOGICAL_NOT,
        VM_OP_LOGICAL_AND,
        VM_OP_LOGICAL_OR,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_BOOL_TYPE, .as.intval= 1};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_boolean_comparators(void) {
    // 1 < 2 and 3 > 4 or 4 <= 5 and 7==6
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        EMPTY_CONST_POOL,
        VM_OP_IPUSH_1,
        VM_OP_IPUSH, 2,
        VM_OP_LT,
        VM_OP_IPUSH, 3,
        VM_OP_IPUSH, 4,
        VM_OP_GT,
        VM_OP_LOGICAL_AND,
        VM_OP_IPUSH, 4,
        VM_OP_IPUSH, 5,
        VM_OP_LE,
        VM_OP_IPUSH, 7,
        VM_OP_IPUSH, 6,
        VM_OP_EQ,
        VM_OP_LOGICAL_OR,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_BOOL_TYPE, .as.boolval= 1};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_if_true_then(void) {
    // if (true) then { 2 + 2 }
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        EMPTY_CONST_POOL,
        VM_OP_ZPUSH_T,
        VM_OP_JZ, 16, 0,
        VM_OP_IPUSH, 2,
        VM_OP_IPUSH, 2,
        VM_OP_ADD,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 4};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_if_false_then(void) {
    // push -1; if (false) then { 2 + 2 }
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        EMPTY_CONST_POOL,
        VM_OP_IPUSH_1N,
        VM_OP_ZPUSH_F,
        VM_OP_JZ, 17, 0,
        VM_OP_IPUSH, 2,
        VM_OP_IPUSH, 2,
        VM_OP_ADD,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= -1};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_if_true_then_else(void) {
    // if (true) then { 1 + 2 } else { 3 + 4 }
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        EMPTY_CONST_POOL,
        VM_OP_ZPUSH_T,
        VM_OP_JZ, 0x12, 0,
        VM_OP_IPUSH_1,
        VM_OP_IPUSH, 2,
        VM_OP_ADD,
        VM_OP_JMP, 0x17, 0,
        VM_OP_IPUSH, 3,
        VM_OP_IPUSH, 4,
        VM_OP_ADD,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 3};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_if_false_then_else(void) {
    // if (false) then { 1 + 2 } else { 3 + 4 }
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        EMPTY_CONST_POOL,
        VM_OP_ZPUSH_F,
        VM_OP_JZ, 0x12, 0,
        VM_OP_IPUSH_1,
        VM_OP_IPUSH, 2,
        VM_OP_ADD,
        VM_OP_JMP, 0x17, 0,
        VM_OP_IPUSH, 3,
        VM_OP_IPUSH, 4,
        VM_OP_ADD,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 7};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_if_false_then_else_if_true(void) {
    // if (false) then { 1 + 2 } else if (true) then { 3 + 4 } else { 5 + 6 }
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        EMPTY_CONST_POOL,
        VM_OP_ZPUSH_F,
        VM_OP_JZ, 0xf, 0,      // if true
        VM_OP_IPUSH_1,
        VM_OP_IPUSH, 2,
        VM_OP_ADD,
        VM_OP_ZPUSH_T,
        VM_OP_JZ, 0x1b, 0,     // else if true
        VM_OP_IPUSH, 3,
        VM_OP_IPUSH, 4,
        VM_OP_ADD,
        VM_OP_JMP, 0x20, 0,
        VM_OP_IPUSH, 5,
        VM_OP_IPUSH, 6,
        VM_OP_ADD,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 7};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_binop_type_check(void) {
    // Can't add 42 + true
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        1, CONST_INT, 1, 42,
        VM_OP_ICONST, 1,
        VM_OP_ZPUSH_T,
        VM_OP_ADD,
        VM_OP_RET
    };

    vm_stack_elem_t stack_top = {.type= 0};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_VM_RUNTIME_ERROR);
}

void test_vm_stack_load_imm(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        // Const pool
        1,
        CONST_INT, 1, 42,
        // Code
        VM_OP_ICONST, 1,
        VM_OP_IPUSH_1N,
        VM_OP_IPUSH_1,
        VM_OP_ADD,
        VM_OP_IPUSH_0,
        VM_OP_ADD,
        VM_OP_ADD,
        VM_OP_RET
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 42};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_stack_inc_dec(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        EMPTY_CONST_POOL,
        VM_OP_IPUSH_0,
        VM_OP_INC,
        VM_OP_INC,
        VM_OP_DEC,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_INT_TYPE, .as.intval= 1};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_string_index(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        1,
        CONST_STRING, 5, 'E', 't', 'h', 'e', 'l',
        VM_OP_SCONST, 1,
        VM_OP_IPUSH, 3,
        VM_OP_ALOAD,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_BYTE_TYPE, .as.byteval= 'e'};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_string_index_out_of_range(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        1,
        CONST_STRING, 5, 'E', 't', 'h', 'e', 'l',
        VM_OP_SCONST, 1,
        VM_OP_IPUSH, 5,
        VM_OP_ALOAD,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= 0};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_VM_COMPILE_ERROR);
}

void test_vm_string_negative_index(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        1,
        CONST_STRING, 5, 'E', 't', 'h', 'e', 'l',
        VM_OP_SCONST, 1,
        VM_OP_IPUSH, 3,
        VM_OP_NEG,
        VM_OP_ALOAD,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= VM_STACK_BYTE_TYPE, .as.byteval= 'h'};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_NO_ERROR);
}

void test_vm_string_negative_index_out_of_range(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        1,
        CONST_STRING, 5, 'E', 't', 'h', 'e', 'l',
        VM_OP_SCONST, 1,
        VM_OP_IPUSH, 6,
        VM_OP_NEG,
        VM_OP_ALOAD,
        VM_OP_RET,
    };

    vm_stack_elem_t stack_top = {.type= 0};
    test_program_primitive(bytes, sizeof(bytes), stack_top, ERR_VM_COMPILE_ERROR);
}

void test_vm_byte_array_alloc(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        EMPTY_CONST_POOL,
        VM_OP_IPUSH, 42,
        VM_OP_AALLOC,
        VM_OP_RET,
    };

    test_program_object(bytes, sizeof(bytes), OBJ_TYPE_BYTEARRAY, ERR_NO_ERROR);
}

void test_vm_byte_array(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        1,
        CONST_BYTEARRAY, 4, 0, 0, 0, 20, 21, 22, 23,
        VM_OP_ACONST, 1,
        VM_OP_RET,
    };

    test_program_object(bytes, sizeof(bytes), OBJ_TYPE_BYTEARRAY, ERR_NO_ERROR);
}

void test_vm_print_int(void) {
    uint8_t bytes[] = {
        'E', 'T', 'H', 'L', 0, 1,
        2,
        CONST_STRING, 5, 'E', 't', 'h', 'e', 'l',
        CONST_BYTEARRAY, 4, 0, 0, 0, 4, 5, 6, 7,
        VM_OP_ZPUSH_T,
        VM_OP_PRINT,
        VM_OP_IPUSH_1,
        VM_OP_PRINT,
        VM_OP_IPUSH, -42,
        VM_OP_PRINT,
        VM_OP_SCONST, 1,
        VM_OP_PRINT,
        VM_OP_ACONST, 2,
        VM_OP_PRINT,
        VM_OP_NIL,
        VM_OP_PRINT,
        VM_OP_RET,
    };

    vm_stack_elem_t no_stack = {.type= 0};
    test_program_primitive(bytes, sizeof(bytes), no_stack, ERR_NO_ERROR);
}

void test_vm(void) {
    RUN_TEST(test_vm_init);
    RUN_TEST(test_vm_load_code);
    RUN_TEST(test_vm_stack_push_byte);
    RUN_TEST(test_vm_stack_push_byte_as_int32);
    RUN_TEST(test_vm_stack_push_int);
    RUN_TEST(test_vm_stack_push);
    RUN_TEST(test_vm_iconst);
    RUN_TEST(test_vm_iconst_negative);
    RUN_TEST(test_vm_sconst);
    RUN_TEST(test_vm_stack_negate);
    RUN_TEST(test_vm_stack_add);
    RUN_TEST(test_vm_stack_sub);
    RUN_TEST(test_vm_stack_mul);
    RUN_TEST(test_vm_stack_div);
    RUN_TEST(test_vm_stack_rem);
    RUN_TEST(test_vm_stack_arith);
    RUN_TEST(test_vm_booleans);
    RUN_TEST(test_vm_non_boolean_booleans);
    RUN_TEST(test_vm_boolean_comparators);
    RUN_TEST(test_vm_if_true_then);
    RUN_TEST(test_vm_if_false_then);
    RUN_TEST(test_vm_if_true_then_else);
    RUN_TEST(test_vm_if_false_then_else);
    RUN_TEST(test_vm_if_false_then_else);
    RUN_TEST(test_vm_if_false_then_else_if_true);
    RUN_TEST(test_vm_binop_type_check);
    RUN_TEST(test_vm_stack_load_imm);
    RUN_TEST(test_vm_stack_inc_dec);
    RUN_TEST(test_vm_string_index);
    RUN_TEST(test_vm_string_index_out_of_range);
    RUN_TEST(test_vm_string_negative_index);
    RUN_TEST(test_vm_string_negative_index_out_of_range);
    RUN_TEST(test_vm_byte_array_alloc);
    RUN_TEST(test_vm_byte_array);
    RUN_TEST(test_vm_print_int);
}
