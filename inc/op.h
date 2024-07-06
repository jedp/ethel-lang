#pragma once

typedef enum {
    VM_OP_NO_SUCH_OP,
    VM_OP_NOP,
    VM_OP_LOADI,
    VM_OP_NEGATE,
    VM_OP_RET,
    VM_OP_MAX,
} vm_op_t;

static const char *op_names[VM_OP_MAX] = {
    [VM_OP_NO_SUCH_OP] = "ERROR_NO_SUCH_OP",
    [VM_OP_NOP] = "NOP",
    [VM_OP_LOADI] = "LOAD INT",
    [VM_OP_NEGATE] = "NEGATE",
    [VM_OP_RET] = "RET",
};
