#include <stdio.h>
#include "../inc/type.h"
#include "../inc/mem.h"
#include "../inc/math.h"
#include "../inc/float.h"
#include "../inc/int.h"
#include "../inc/str.h"

obj_t *int_hash(obj_t *obj, obj_varargs_t *args) {
    (void) args;

    // 32-bit int is its own 32-bit hash.
    return int_obj(obj->intval);
}

obj_t *int_copy(obj_t *obj, obj_varargs_t *args) {
    (void) args;

    return int_obj(obj->intval);
}

obj_t *int_to_int(obj_t *obj, obj_varargs_t *args) {
    (void) args;

    return obj;
}

obj_t *int_to_string(obj_t *obj, obj_varargs_t *args) {
    (void) args;

    int n = obj->intval;
    int digits = 0;
    if (n < 0) digits++; // Sign.
    int na = abs(n);
    while (na > 0) {
        na /= 10;
        digits++;
    }

    // Edge case
    if (digits == 0) {
        return string_obj(c_str_to_bytearray("0"));
    }

    bytearray_t *a = bytearray_alloc(digits);

    na = abs(n);
    int i = digits - 1;
    while (na > 0) {
        a->data[i] = (na % 10) + '0';
        na /= 10;
        i--;
    }
    if (n < 0) a->data[0] = '-';

    return string_obj(a);
}

obj_t *int_to_float(obj_t *obj, obj_varargs_t *args) {
    (void) args;

    return float_obj((float) obj->intval);
}

obj_t *int_to_byte(obj_t *obj, obj_varargs_t *args) {
    (void) args;

    return byte_obj((byte) obj->intval & 0xff);
}

obj_t *int_eq(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return boolean_obj(False);
    obj_t *arg = args->arg;

    switch (TYPEOF(arg)) {
        case TYPE_INT:
            return boolean_obj((obj->intval == arg->intval) ? True : False);
        case TYPE_FLOAT:
            return boolean_obj((obj->intval == arg->floatval) ? True : False);
        case TYPE_BYTE:
            return boolean_obj((obj->intval == (int) arg->byteval) ? True : False);
        default:
            printf("Cannot compare for equality between %s and %s.\n",
                   type_names[TYPEOF(obj)], type_names[TYPEOF(arg)]);
            return boolean_obj(False);
    }
}

obj_t *int_ne(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return boolean_obj(False);
    obj_t *arg = args->arg;
    if (TYPEOF(arg) != TYPE_INT &&
        TYPEOF(arg) != TYPE_FLOAT &&
        TYPEOF(arg) != TYPE_BYTE) {
        printf("Cannot compare for equality between %s and %s.\n",
               type_names[TYPEOF(obj)], type_names[TYPEOF(arg)]);
        return boolean_obj(False);
    }

    obj_t *eq = int_eq(obj, args);
    return boolean_obj((eq->boolval == True) ? False : True);
}

obj_t *int_lt(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return boolean_obj(False);
    obj_t *arg = args->arg;

    switch (TYPEOF(arg)) {
        case TYPE_INT:
            return boolean_obj((obj->intval < arg->intval) ? True : False);
        case TYPE_FLOAT:
            return boolean_obj((obj->intval < arg->floatval) ? True : False);
        case TYPE_BYTE:
            return boolean_obj(
                    ((uint32_t) obj->intval < (uint32_t) arg->byteval) ? True : False);
        default:
            printf("Cannot compare %s and %s.\n",
                   type_names[TYPEOF(obj)], type_names[TYPEOF(arg)]);
            return boolean_obj(False);
    }
}

obj_t *int_gt(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return boolean_obj(False);
    obj_t *arg = args->arg;

    switch (TYPEOF(arg)) {
        case TYPE_INT:
            return boolean_obj((obj->intval > arg->intval) ? True : False);
        case TYPE_FLOAT:
            return boolean_obj((obj->intval > arg->floatval) ? True : False);
        case TYPE_BYTE:
            return boolean_obj(
                    ((uint32_t) obj->intval > (uint32_t) arg->byteval) ? True : False);
        default:
            printf("Cannot compare %s and %s.\n",
                   type_names[TYPEOF(obj)], type_names[TYPEOF(arg)]);
            return boolean_obj(False);
    }
}

obj_t *int_le(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return boolean_obj(False);
    obj_t *arg = args->arg;
    if (TYPEOF(arg) != TYPE_INT &&
        TYPEOF(arg) != TYPE_FLOAT &&
        TYPEOF(arg) != TYPE_BYTE) {
        printf("Cannot compare for equality between %s and %s.\n",
               type_names[TYPEOF(obj)], type_names[TYPEOF(arg)]);
        return boolean_obj(False);
    }

    obj_t *gt = int_gt(obj, args);
    return boolean_obj((gt->boolval == True) ? False : True);
}

obj_t *int_ge(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return boolean_obj(False);
    obj_t *arg = args->arg;
    if (TYPEOF(arg) != TYPE_INT &&
        TYPEOF(arg) != TYPE_FLOAT &&
        TYPEOF(arg) != TYPE_BYTE) {
        printf("Cannot compare for equality between %s and %s.\n",
               type_names[TYPEOF(obj)], type_names[TYPEOF(arg)]);
        return boolean_obj(False);
    }

    obj_t *lt = int_lt(obj, args);
    return boolean_obj((lt->boolval == True) ? False : True);
}

obj_t *int_math(obj_t *obj,
                obj_varargs_t *args,
                static_method_ident_t method_id) {
    if (args == NULL || args->arg == NULL) return obj;
    obj_t *arg = args->arg;
    static_method m_cast = get_static_method(TYPEOF(arg), METHOD_TO_INT);
    if (m_cast == NULL) {
        printf("Cannot do math with %s and %s",
               type_names[TYPEOF(arg)], type_names[TYPEOF(obj)]);
        return obj;
    }

    // Convert to float if other arg is a float.
    if (TYPEOF(arg) == TYPE_FLOAT) {
        return float_math(float_obj((float) obj->intval), args, method_id);
    }

    switch (method_id) {
        case METHOD_ADD:
            return int_obj(obj->intval + m_cast(arg, NULL)->intval);
        case METHOD_SUB:
            return int_obj(obj->intval - m_cast(arg, NULL)->intval);
        case METHOD_MUL:
            return int_obj(obj->intval * m_cast(arg, NULL)->intval);
        case METHOD_DIV: {
            int divisor = m_cast(arg, NULL)->intval;
            if (divisor == 0) {
                return error_obj(ERR_DIVISION_BY_ZERO);
            }
            return int_obj(obj->intval / divisor);
        }
        case METHOD_MOD: {
            int divisor = m_cast(arg, NULL)->intval;
            if (divisor == 0) {
                return error_obj(ERR_DIVISION_BY_ZERO);
            }
            return int_obj(obj->intval % divisor);
        }
        default:
            printf("method_id %d not implemented!\n", method_id);
            return obj;
    }
}

obj_t *int_add(obj_t *obj, obj_varargs_t *args) {
    return int_math(obj, args, METHOD_ADD);
}

obj_t *int_sub(obj_t *obj, obj_varargs_t *args) {
    return int_math(obj, args, METHOD_SUB);
}

obj_t *int_mul(obj_t *obj, obj_varargs_t *args) {
    return int_math(obj, args, METHOD_MUL);
}

obj_t *int_div(obj_t *obj, obj_varargs_t *args) {
    return int_math(obj, args, METHOD_DIV);
}

obj_t *int_mod(obj_t *obj, obj_varargs_t *args) {
    return int_math(obj, args, METHOD_MOD);
}

obj_t *int_as(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return boolean_obj(False);
    obj_t *type_arg = args->arg;

    switch (TYPEOF(type_arg)) {
        case TYPE_INT:
            return obj;
        case TYPE_FLOAT:
            return float_obj((float) obj->intval);
        case TYPE_BYTE:
            return byte_obj((uint32_t) obj->intval & 0xff);
        case TYPE_STRING:
            return int_to_string(obj, NULL);
        case TYPE_BOOLEAN:
            return boolean_obj((obj->intval != 0) ? True : False);
        default:
            printf("Cannot cast %s to type %s.\n",
                   type_names[TYPEOF(obj)], type_names[TYPEOF(type_arg)]);
            return boolean_obj(False);
    }
}

obj_t *int_abs(obj_t *obj, obj_varargs_t /* Ignored */ *args) {
    return int_obj(abs(obj->intval));
}

obj_t *int_neg(obj_t *obj, obj_varargs_t /* Ignored */ *args) {
    return int_obj(-obj->intval);
}

obj_t *int_bitwise_and(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return obj;
    obj_t *arg = args->arg;

    switch (TYPEOF(arg)) {
        case TYPE_INT:
            return int_obj(obj->intval & arg->intval);
        case TYPE_BYTE:
            return int_obj(obj->intval & (unsigned char) arg->byteval);
        default:
            printf("Cannot perform bitwise and with %s and %s.\n",
                   type_names[TYPEOF(obj)], type_names[TYPEOF(arg)]);
            return nil_obj();
    }
}

obj_t *int_bitwise_or(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return obj;
    obj_t *arg = args->arg;

    switch (TYPEOF(arg)) {
        case TYPE_INT:
            return int_obj(obj->intval | arg->intval);
        case TYPE_BYTE:
            return int_obj(obj->intval | (unsigned char) arg->byteval);
        default:
            printf("Cannot perform bitwise or with %s and %s.\n",
                   type_names[TYPEOF(obj)], type_names[TYPEOF(arg)]);
            return nil_obj();
    }
}

obj_t *int_bitwise_xor(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return obj;
    obj_t *arg = args->arg;

    switch (TYPEOF(arg)) {
        case TYPE_INT:
            return int_obj(obj->intval ^ arg->intval);
        case TYPE_BYTE:
            return int_obj(obj->intval ^ (unsigned char) arg->byteval);
        default:
            printf("Cannot perform bitwise xor with %s and %s.\n",
                   type_names[TYPEOF(obj)], type_names[TYPEOF(arg)]);
            return nil_obj();
    }
}

obj_t *int_bitwise_not(obj_t *obj, obj_varargs_t /* Ignored */ *args) {
    return int_obj(~obj->intval);
}

obj_t *int_bitwise_shl(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return obj;
    obj_t *arg = args->arg;

    switch (TYPEOF(arg)) {
        case TYPE_INT:
            return int_obj(obj->intval << (unsigned int) arg->intval);
        case TYPE_BYTE:
            return int_obj(obj->intval << (unsigned char) arg->byteval);
        default:
            printf("Cannot perform bitwise and with %s and %s.\n",
                   type_names[TYPEOF(obj)], type_names[TYPEOF(arg)]);
            return nil_obj();
    }
}

obj_t *int_bitwise_shr(obj_t *obj, obj_varargs_t *args) {
    if (args == NULL || args->arg == NULL) return obj;
    obj_t *arg = args->arg;

    switch (TYPEOF(arg)) {
        case TYPE_INT:
            return int_obj(obj->intval >> (unsigned int) arg->intval);
        case TYPE_BYTE:
            return int_obj(obj->intval >> (unsigned char) arg->byteval);
        default:
            printf("Cannot perform bitwise and with %s and %s.\n",
                   type_names[TYPEOF(obj)], type_names[TYPEOF(arg)]);
            return nil_obj();
    }
}

static_method get_int_static_method(static_method_ident_t method_id) {
    switch (method_id) {
        case METHOD_HASH:
            return int_hash;
        case METHOD_COPY:
            return int_copy;
        case METHOD_TO_INT:
            return int_to_int;
        case METHOD_TO_STRING:
            return int_to_string;
        case METHOD_TO_FLOAT:
            return int_to_float;
        case METHOD_TO_BYTE:
            return int_to_byte;
        case METHOD_ABS:
            return int_abs;
        case METHOD_NEG:
            return int_neg;
        case METHOD_EQ:
            return int_eq;
        case METHOD_NE:
            return int_ne;
        case METHOD_LT:
            return int_lt;
        case METHOD_LE:
            return int_le;
        case METHOD_GT:
            return int_gt;
        case METHOD_GE:
            return int_ge;
        case METHOD_ADD:
            return int_add;
        case METHOD_SUB:
            return int_sub;
        case METHOD_MUL:
            return int_mul;
        case METHOD_DIV:
            return int_div;
        case METHOD_MOD:
            return int_mod;
        case METHOD_BITWISE_AND:
            return int_bitwise_and;
        case METHOD_BITWISE_OR:
            return int_bitwise_or;
        case METHOD_BITWISE_XOR:
            return int_bitwise_xor;
        case METHOD_BITWISE_NOT:
            return int_bitwise_not;
        case METHOD_BITWISE_SHL:
            return int_bitwise_shl;
        case METHOD_BITWISE_SHR:
            return int_bitwise_shr;
        case METHOD_CAST:
            return int_as;
        default:
            return NULL;
    }
}
