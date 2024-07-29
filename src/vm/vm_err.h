#pragma once

typedef enum {
    VM_ERR_NO_ERROR,
    VM_ERR_COMPILE_ERROR,
    VM_ERR_LOAD_ERROR,
    VM_ERR_RUNTIME_ERROR,
    VM_ERR_STACK_OVERFLOW,
    VM_ERR_OUT_OF_MEMORY,
} vm_err_t;

static const char *vm_err_names[] = {
    [VM_ERR_NO_ERROR] = "VM OK",
    [VM_ERR_COMPILE_ERROR]= "Compile error",
    [VM_ERR_LOAD_ERROR] = "Error loading bytecode",
    [VM_ERR_RUNTIME_ERROR]= "Runtime error",
    [VM_ERR_STACK_OVERFLOW]= "VM Stack Overflow",
    [VM_ERR_OUT_OF_MEMORY] = "Out of memory",
};
