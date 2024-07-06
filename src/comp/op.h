#pragma once

typedef enum {
    VM_OP_NO_SUCH_OP,
    VM_OP_NOP,
    VM_OP_LOADI,
    VM_OP_LOADI_1N,
    VM_OP_LOADI_0,
    VM_OP_LOADI_1,
    VM_OP_NEGATE,
    VM_OP_ADD,
    VM_OP_SUB,
    VM_OP_MUL,
    VM_OP_DIV,
    VM_OP_REM,
    VM_OP_INC,
    VM_OP_DEC,
    VM_OP_RET,
    VM_OP_MAX,
} vm_op_t;

static const char *op_names[VM_OP_MAX] = {
    [VM_OP_NO_SUCH_OP] = "ERROR_NO_SUCH_OP",
    [VM_OP_NOP] = "NOP",
    [VM_OP_LOADI] = "LOAD INT",
    [VM_OP_LOADI_1N] = "LOAD INT IMM [-1]",
    [VM_OP_LOADI_0] = "LOAD INT IMM [0]",
    [VM_OP_LOADI_1] = "LOAD INT IMM [1]",
    [VM_OP_NEGATE] = "NEGATE",
    [VM_OP_ADD] = "ADD",
    [VM_OP_SUB] = "SUB",
    [VM_OP_MUL] = "MUL",
    [VM_OP_DIV] = "DIV",
    [VM_OP_REM] = "REM",
    [VM_OP_INC] = "INC",
    [VM_OP_DEC] = "DEC",
    [VM_OP_RET] = "RET",
};
