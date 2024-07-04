#pragma once

typedef enum {
    VM_OP_NOP,
    VM_OP_RET,
    VM_OP_MAX,
} vm_op_t;

static const char *op_names[VM_OP_MAX] = {
    [VM_OP_NOP] = "NOP",
    [VM_OP_RET] = "RET",
};
