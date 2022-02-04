#include <stdio.h>
#include "../inc/type.h"
#include "../inc/math.h"
#include "../inc/mem.h"
#include "../inc/rand.h"
#include "../inc/obj.h"
#include "../inc/arr.h"

static boolean _eq(bytearray_t *a, bytearray_t *b) {
    if (a->size != b->size) return False;

    for (size_t i = 0; i < a->size; i++) {
        if (a->data[i] != b->data[i]) {
            return False;
        }
    }

    return True;
}

static obj_t *_arr_slice(obj_t *obj, int start, int end) {
    if (end > obj->bytearray->size) end = obj->bytearray->size;

    if (end < 0 ||
        start < 0 ||
        end < start) {
        printf("Bad range for slice\n");
        return bytearray_obj(0, NULL);
    }

    size_t len = end - start;
    obj_t *new = bytearray_obj(len, NULL);
    for (size_t i = 0; i < len; i++) {
        new->bytearray->data[i] = obj->bytearray->data[start + i];
    }

    return new;
}

static byte obj_to_byte(obj_t *obj) {
    switch (TYPEOF(obj)) {
        case TYPE_BYTE:
            return obj->byteval;
        case TYPE_INT: {
            if (obj->intval > 255 || obj->intval < -127) return 0xFF;
            return (byte) obj->intval;
        }
        default:
            printf("Don't know how to convert %s to byte.\n", type_names[TYPEOF(obj)]);
            return 0x0;
    }
}

obj_t *arr_hash(obj_t *obj, obj_varargs_t /* Ignored */ *args) {
    uint32_t temp;
    byte b;

    if (obj->bytearray->size == 0) {
        return nil_obj();
    }

    /*
     * FNV (Fowler, Noll, Vo), the non-cryptographic hash function FNV-1a
     * for 32-bit hashes returning a 32-bit integer.
     */
    temp = FNV32Basis;
    size_t i = 0;
    while (i < obj->bytearray->size) {
        b = obj->bytearray->data[i];
        temp = FNV32Prime * (temp ^ b);
        i++;
    }

    return int_obj(temp);
}

obj_t *arr_size(obj_t *obj, obj_varargs_t /* Ignored */ *args) {
    return int_obj(obj->bytearray->size);
}

obj_t *arr_copy(obj_t *obj, obj_varargs_t *args) {
    return bytearray_obj(obj->bytearray->size, obj->bytearray->data);
}

obj_t *_arr_get(obj_t *obj, int i) {
    if (i < 0 || i >= obj->bytearray->size) {
        return nil_obj();
    }

    return byte_obj(obj->bytearray->data[i]);
}

obj_t *arr_get(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) {
        printf("Null arg to get()\n");
        return nil_obj();
    }
    // Get first arg as int offset.
    obj_t *arg = args->arg;
    if (TYPEOF(arg) != TYPE_INT) {
        return nil_obj();
    }
    return _arr_get(obj, arg->intval);
}

obj_t *arr_set(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) {
        printf("Null arg to set()\n");
        return nil_obj();
    }
    // Get first arg as int offset.
    obj_t *a = args->arg;
    if (TYPEOF(a) != TYPE_INT) {
        printf("Int offset required.\n");
        return nil_obj();
    }
    int i = a->intval;
    if (i < 0 || i >= obj->bytearray->size) {
        printf("Out of bounds\n");
        return nil_obj();
    }

    // Get second arg as byte to set.
    obj_t *b = args->next->arg;
    if (b == NULL) {
        printf("Null arg for value.\n");
        return nil_obj();
    }

    byte val = obj_to_byte(b);
    obj->bytearray->data[i] = val;
    return byte_obj(val);
}

obj_t *arr_contains(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) {
        printf("Null arg to contains()\n");
        return nil_obj();
    }

    obj_t *arg = args->arg;
    if (TYPEOF(arg) != TYPE_INT &&
        TYPEOF(arg) != TYPE_BYTE) {
        return nil_obj();
    }

    if (TYPEOF(arg) == TYPE_INT && arg->intval > 255) {
        printf("Integer too large for byte");
        return nil_obj();
    }

    byte b;
    if (TYPEOF(arg) == TYPE_BYTE) {
        b = arg->byteval;
    } else if (TYPEOF(arg) == TYPE_INT) {
        b = (byte) arg->intval & 0xff;
    } else {
        printf("This can't happen");
        return nil_obj();
    }

    for (size_t i = 0; i < obj->bytearray->size; i++) {
        if (obj->bytearray->data[i] == b) {
            return boolean_obj(True);
        }
    }

    return boolean_obj(False);
}

obj_t *arr_eq(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) {
        printf("Null arg to eq()\n");
        return nil_obj();
    }

    obj_t *other = args->arg;
    if (TYPEOF(other) != TYPE_STRING &&
        TYPEOF(other) != TYPE_BYTEARRAY) {
        return nil_obj();
    }

    return boolean_obj(_eq(obj->bytearray, other->bytearray));
}

obj_t *arr_ne(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) {
        printf("Null arg to ne()\n");
        return nil_obj();
    }

    obj_t *other = args->arg;
    if (TYPEOF(other) != TYPE_STRING &&
        TYPEOF(other) != TYPE_BYTEARRAY) {
        return nil_obj();
    }

    return boolean_obj(!_eq(obj->bytearray, other->bytearray));
}

obj_t *arr_slice(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) {
        printf("Null arg to slice()\n");
        return nil_obj();
    }

    obj_t *start_arg = args->arg;
    if (TYPEOF(start_arg) != TYPE_INT) return nil_obj();

    obj_t *end_arg = args->next->arg;
    if (TYPEOF(end_arg) != TYPE_INT) return nil_obj();

    return _arr_slice(obj, start_arg->intval, end_arg->intval);
}

obj_t *arr_random_choice(obj_t *obj, obj_varargs_t *args) {
    size_t len = obj->bytearray->size;
    if (len < 1) {
        printf("Empty string.\n");
        return nil_obj();
    }
    return byte_obj(obj->bytearray->data[rand32() % len]);
}

static obj_t *iter_next(obj_iter_t *iterable) {
    size_t i;

    switch (iterable->state) {
        case ITER_NOT_STARTED:
            i = 0;
            iterable->state_obj->intval = i;
            iterable->state = ITER_ITERATING;
            // Fall through to ITERATING.

        case ITER_ITERATING:
            i = iterable->state_obj->intval;
            obj_t *next_val = _arr_get(iterable->obj, i);
            if (i >= iterable->obj->bytearray->size) {
                iterable->state = ITER_STOPPED;
                return nil_obj();
            }
            iterable->state_obj->intval = i + 1;
            return next_val;

        case ITER_STOPPED:
            return nil_obj();

        default:
            printf("Unexpected iteration state enum! %d\n", iterable->state);
            return nil_obj();
    }
}

obj_t *arr_iterator(obj_t *obj, obj_varargs_t *args) {
    obj_t *state = int_obj(0);
    return iterator_obj(obj, state, iter_next);
}

static_method get_arr_static_method(static_method_ident_t method_id) {
    switch (method_id) {
        case METHOD_HASH:
            return arr_hash;
        case METHOD_COPY:
            return arr_copy;
        case METHOD_LENGTH:
            return arr_size;
        case METHOD_GET:
            return arr_get;
        case METHOD_CONTAINS:
            return arr_contains;
        case METHOD_EQ:
            return arr_eq;
        case METHOD_NE:
            return arr_ne;
        case METHOD_SLICE:
            return arr_slice;
        case METHOD_RANDOM_CHOICE:
            return arr_random_choice;
        case METHOD_ITERATOR:
            return arr_iterator;
        default:
            return NULL;
    }
}
