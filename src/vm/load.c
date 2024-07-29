#include <stdint.h>
#include <stddef.h>
#include <printf.h>
#include "../comp/comp.h"
#include "vm.h"
#include "load.h"

static uint32_t vm_interp_header(vm_t *vm) {
    uint32_t offset = 0;
    uint8_t *bytes = vm->bytecode;

    // Magic
    if (!(bytes[offset++] == 'E' &&
          bytes[offset++] == 'T' &&
          bytes[offset++] == 'H' &&
          bytes[offset++] == 'L')) {
        return 0;
    }
    // Version
    if (!(bytes[offset++] == 0 && bytes[offset++] == 1)) {
        return 0;
    }

    uint8_t num_consts = bytes[offset++];
    if (num_consts == 0) {
        return offset;
    }

    for (uint8_t i = 1; i <= num_consts; i++) {
        const_type_t type = bytes[offset++];
        val_t v;
        uint8_t k;
        switch (type) {
            case CONST_INT: {
                // Ints are packed into as few bytes as possible,
                // least-significant byte first.
                // Extend sign bit if necessary.
                uint32_t uintval = 0;
                uint8_t intsize = bytes[offset++];
                uintval |= (bytes[offset++]) & 0xff;
                if (intsize > 1)
                    uintval |= (bytes[offset++] << 8) & 0xff00;
                if (intsize > 2)
                    uintval |= (bytes[offset++] << 16) & 0xff0000;
                if (intsize > 3)
                    uintval |= (bytes[offset++] << 24) & 0xff000000;
                if (intsize > 4)
                    return 0;
                // Extend sign bit.
                if (intsize < 4 && uintval & (1 << ((intsize - 1) * 8 + 7))) {
                    uintval |= (0xffffffff << intsize * 8);
                }
                v.type = VAL_TYPE_INT;
                v.as.intval = (int) uintval;
                printf("PUT CONST %d\n", uintval);
                vm_put_const(vm, v, &k);
                break;
            }
            case CONST_STRING: {
                // TODO enforce string max size?
                uint8_t strlen = bytes[offset++];
                v.type = VAL_TYPE_OBJ;
                v.as.objval = (obj_t *) obj_str_new((const char *) &bytes[offset], strlen);
                printf("PUT CONST %s\n", obj_str_to_c((obj_str_t *) v.as.objval));
                vm_put_const(vm, v, &k);
                offset += strlen;
                break;
            }
            case CONST_BYTEARRAY: {
                uint32_t arrlen = 0;
                // First four bytes are array length.
                arrlen |= (bytes[offset++]) & 0xff;
                arrlen |= (bytes[offset++] << 8) & 0xff00;
                arrlen |= (bytes[offset++] << 16) & 0xff0000;
                arrlen |= (bytes[offset++] << 24) & 0xff000000;
                v.type = VAL_TYPE_OBJ;
                v.as.objval = (obj_t *) obj_arr_new((const uint8_t *) &bytes[offset], arrlen);
                vm_put_const(vm, v, &k);
                offset += arrlen;
                break;
            }
            default:
                runtime_error(vm, "We don't handle const type %d yet!", type);
                return 0;
        }
        if (k != i) {
            runtime_error(vm, "Created const index %d, but bytecode index is %d\n", k, i);
            return 0;
        }
    }

    return offset;
}

vm_err_t vm_load_bytecode(vm_t *vm, const uint8_t *bytecode, uint32_t length) {
    vm_init(vm, bytecode, length);

    uint32_t code_start = vm_interp_header(vm);
    if (code_start == 0) {
        return VM_ERR_LOAD_ERROR;
    }
    vm->code_start = code_start;
    vm->pc = vm->bytecode + code_start;
    vm->bytecode_size = length;
    return VM_ERR_NO_ERROR;
}
