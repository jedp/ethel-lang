#include <stdio.h>
#include "../inc/arr.h"
#include "../inc/byte.h"
#include "../inc/obj.h"
#include "../inc/str.h"
#include "../inc/type.h"

obj_t *byte_hash(obj_t *obj, obj_method_args_t *args) {
  // 8-bit int is its own 32-bit hash.
  return int_obj((uint32_t) obj->byteval);
}

obj_t *byte_copy(obj_t *obj, obj_method_args_t *args) {
  return byte_obj(obj->byteval);
}

obj_t *byte_to_int(obj_t *obj, obj_method_args_t *args) {
  return int_obj((uint32_t) obj->byteval);
}

obj_t *byte_to_string(obj_t *obj, obj_method_args_t *args) {
  byte c = obj->byteval;
  if (c >= ' ' && c <= '~') {
    bytearray_t *a = bytearray_alloc(3);
    a->data[0] = '\'';
    a->data[1] = c;
    a->data[2] = '\'';
    return string_obj(a);
  }

  bytearray_t *a = bytearray_alloc(4);
  a->data[0] = '0';
  a->data[1] = 'x';
  a->data[2] = hex_char((c & 0xf0) >> 4);
  a->data[3] = hex_char(c & 0xf);
  return string_obj(a);
}

obj_t *byte_to_float(obj_t *obj, obj_method_args_t *args) {
  return float_obj((float) obj->byteval);
}

obj_t *byte_to_byte(obj_t *obj, obj_method_args_t *args) {
  return obj;
}

obj_t *byte_math(obj_t *obj, obj_method_args_t *args, static_method_ident_t method_id) {
  if (args == NULL || args->arg == NULL) return obj;
  obj_t *arg = args->arg;
  static_method m_cast = get_static_method(arg->type, METHOD_TO_BYTE);
  if (m_cast == NULL) {
    printf("Cannot do math with %s and %s",
        obj_type_names[arg->type], obj_type_names[obj->type]);
    return obj;
  }

  switch (method_id) {
    case METHOD_ADD:
      return byte_obj((byte) ((obj->byteval + m_cast(arg, NULL)->byteval) & 0xff));
    case METHOD_SUB:
      return byte_obj((byte) ((obj->byteval - m_cast(arg, NULL)->byteval) & 0xff));
    case METHOD_MUL:
      return byte_obj((byte) ((obj->byteval * m_cast(arg, NULL)->byteval) & 0xff));
    case METHOD_DIV: {
      byte divisor = m_cast(arg, NULL)->byteval;
      if (divisor == 0) {
        return error_obj(ERR_DIVISION_BY_ZERO);
      }
      return byte_obj(obj->byteval / divisor);
    }
    case METHOD_MOD: {
      byte divisor = m_cast(arg, NULL)->byteval;
      if (divisor == 0) {
        return error_obj(ERR_DIVISION_BY_ZERO);
      }
      return byte_obj(obj->byteval % divisor);
    }
    default:
      printf("method_id %d not implemented!\n", method_id);
      return obj;
  }
  return byte_obj((byte) ((obj->byteval + m_cast(arg, NULL)->byteval) & 0xff));
}

obj_t *byte_add(obj_t *obj, obj_method_args_t *args) {
  return byte_math(obj, args, METHOD_ADD);
}

obj_t *byte_sub(obj_t *obj, obj_method_args_t *args) {
  return byte_math(obj, args, METHOD_SUB);
}

obj_t *byte_mul(obj_t *obj, obj_method_args_t *args) {
  return byte_math(obj, args, METHOD_MUL);
}

obj_t *byte_div(obj_t *obj, obj_method_args_t *args) {
  return byte_math(obj, args, METHOD_DIV);
}

obj_t *byte_mod(obj_t *obj, obj_method_args_t *args) {
  return byte_math(obj, args, METHOD_MOD);
}

obj_t *byte_eq(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_BYTE:
      return boolean_obj((obj->byteval == arg->byteval) ? True : False);
    case TYPE_INT:
      return boolean_obj(
          ((uint8_t) obj->byteval == (uint32_t) arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
            obj_type_names[TYPE_BYTE], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *byte_ne(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_BYTE && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           obj_type_names[obj->type], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *eq = byte_eq(obj, args);
  return boolean_obj(eq->boolval == True ? False : True);
}

obj_t *byte_lt(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_BYTE:
      return boolean_obj((obj->byteval < arg->byteval) ? True : False);
    case TYPE_INT:
      return boolean_obj(
          ((uint8_t) obj->byteval < (uint32_t) arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
            obj_type_names[obj->type], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *byte_gt(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_BYTE:
      return boolean_obj((obj->byteval > arg->byteval) ? True : False);
    case TYPE_INT:
      return boolean_obj(
          ((uint8_t) obj->byteval > (uint32_t) arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
            obj_type_names[obj->type], obj_type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *byte_ge(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_BYTE && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           obj_type_names[obj->type], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *lt = byte_lt(obj, args);
  return boolean_obj(lt->boolval == True ? False : True);
}

obj_t *byte_le(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_BYTE && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           obj_type_names[obj->type], obj_type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *gt = byte_gt(obj, args);
  return boolean_obj(gt->boolval == True ? False : True);
}

obj_t *byte_as(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *type_arg = args->arg;

  switch (type_arg->type) {
    case TYPE_BYTE: return obj;
    case TYPE_INT: return int_obj((unsigned int) obj->byteval);
    case TYPE_BOOLEAN: return boolean_obj((obj->byteval == 't') ? 1 : 0);
    case TYPE_STRING: {
      bytearray_t *a = bytearray_alloc(1);
      a->data[0] = obj->byteval;
      return string_obj(a);
    }
  }

  printf("Cannot cast %s to %s.\n",
          obj_type_names[obj->type], obj_type_names[type_arg->type]);
  return nil_obj();

}

obj_t *byte_bitwise_and(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return obj;
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_INT: return byte_obj((unsigned char) obj->byteval & arg->intval);
    case TYPE_BYTE: return byte_obj(obj->byteval & arg->byteval);
    default:
      printf("Cannot perform bitwise and with %s and %s.\n",
             obj_type_names[obj->type], obj_type_names[arg->type]);
      return nil_obj();
  }
}

obj_t *byte_bitwise_or(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return obj;
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_INT: return byte_obj((unsigned char) obj->byteval | arg->intval);
    case TYPE_BYTE: return byte_obj(obj->byteval | arg->byteval);
    default:
      printf("Cannot perform bitwise or with %s and %s.\n",
             obj_type_names[obj->type], obj_type_names[arg->type]);
      return nil_obj();
  }
}

obj_t *byte_bitwise_xor(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return obj;
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_INT: return byte_obj((unsigned char) obj->byteval ^ arg->intval);
    case TYPE_BYTE: return byte_obj(obj->byteval ^ arg->byteval);
    default:
      printf("Cannot perform bitwise xor with %s and %s.\n",
             obj_type_names[obj->type], obj_type_names[arg->type]);
      return nil_obj();
  }
}

obj_t *byte_bitwise_not(obj_t *obj, obj_method_args_t /* Ignored */ *args) {
  return byte_obj(~obj->byteval);
}

obj_t *byte_bitwise_shl(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return obj;
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_INT: return byte_obj((unsigned char) obj->byteval << (unsigned int) arg->intval);
    case TYPE_BYTE: return byte_obj(obj->byteval << arg->byteval);
    default:
      printf("Cannot perform bitwise and with %s and %s.\n",
             obj_type_names[obj->type], obj_type_names[arg->type]);
      return nil_obj();
  }
}

obj_t *byte_bitwise_shr(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return obj;
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_INT: return byte_obj((unsigned char) obj->byteval >> (unsigned int) arg->intval);
    case TYPE_BYTE: return byte_obj(obj->byteval >> arg->byteval);
    default:
      printf("Cannot perform bitwise and with %s and %s.\n",
             obj_type_names[obj->type], obj_type_names[arg->type]);
      return nil_obj();
  }
}

static_method get_byte_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return byte_hash;
    case METHOD_COPY: return byte_copy;
    case METHOD_TO_STRING: return byte_to_string;
    case METHOD_TO_INT: return byte_to_int;
    case METHOD_TO_FLOAT: return byte_to_float;
    case METHOD_TO_BYTE: return byte_to_byte;
    case METHOD_EQ: return byte_eq;
    case METHOD_NE: return byte_ne;
    case METHOD_GT: return byte_gt;
    case METHOD_LT: return byte_lt;
    case METHOD_GE: return byte_ge;
    case METHOD_LE: return byte_le;
    case METHOD_BITWISE_AND: return byte_bitwise_and;
    case METHOD_BITWISE_OR: return byte_bitwise_or;
    case METHOD_BITWISE_XOR: return byte_bitwise_xor;
    case METHOD_BITWISE_NOT: return byte_bitwise_not;
    case METHOD_BITWISE_SHL: return byte_bitwise_shl;
    case METHOD_BITWISE_SHR: return byte_bitwise_shr;
    case METHOD_ADD: return byte_add;
    case METHOD_SUB: return byte_sub;
    case METHOD_MUL: return byte_mul;
    case METHOD_DIV: return byte_div;
    case METHOD_MOD: return byte_mod;
    case METHOD_CAST: return byte_as;
    default: return NULL;
  }
}

