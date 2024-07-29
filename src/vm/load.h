#pragma once

#include <stdint.h>

vm_err_t vm_load_bytecode(vm_t *vm, const uint8_t *bytes, uint32_t length);
