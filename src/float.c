#include <stdio.h>
#include "../inc/mem.h"
#include "../inc/float.h"
#include "../inc/str.h"
#include "../inc/obj.h"
#include "../inc/type.h"

obj_t *float_hash(obj_t *obj, obj_method_args_t *args) {
  // 32-bit float is its own 32-bit hash value.
  uint32_t *ip = (uint32_t *) &(obj->floatval);
  return int_obj(*ip);
}

obj_t *float_copy(obj_t *obj, obj_method_args_t *args) {
  return float_obj(obj->floatval);
}

obj_t *float_to_int(obj_t *obj, obj_method_args_t *args) {
  return int_obj((int) obj->floatval);
}

obj_t *float_to_string(obj_t *obj, obj_method_args_t *args) {
  // TODO so lazy. Twiddle those bits, shed a dependency.
  float n = obj->floatval;
  int len = snprintf(NULL, 0, "%f", n);
  char* s = mem_alloc(len + 1);
  snprintf(s, len + 1, "%f", n);
  return string_obj(c_str_to_bytearray(s));
}

obj_t *float_to_float(obj_t *obj, obj_method_args_t *args) {
  return obj;
}

obj_t *float_to_byte(obj_t *obj, obj_method_args_t *args) {
  return byte_obj((byte) ((int) obj->floatval & 0xff));
}

obj_t *float_abs(obj_t *obj, obj_method_args_t *args) {
  return float_obj((obj->floatval < 0) ? 1-obj->floatval : obj->floatval);
}

obj_t *float_neg(obj_t *obj, obj_method_args_t *args) {
  return float_obj(-obj->floatval);
}

obj_t *float_math(obj_t *obj,
                  obj_method_args_t *args,
                  static_method_ident_t method_id) {
  if (args == NULL || args->arg == NULL) return obj;
  obj_t *arg = args->arg;
  static_method m_cast = get_static_method(arg->type, METHOD_TO_FLOAT);
  if (m_cast == NULL) {
    printf("Cannot do math with %s and %s",
        type_names[arg->type], type_names[obj->type]);
    return obj;
  }

  switch (method_id) {
    case METHOD_ADD:
      return float_obj(obj->floatval + m_cast(arg, NULL)->floatval);
    case METHOD_SUB:
      return float_obj(obj->floatval - m_cast(arg, NULL)->floatval);
    case METHOD_MUL:
      return float_obj(obj->floatval * m_cast(arg, NULL)->floatval);
    case METHOD_DIV: {
      float divisor = m_cast(arg, NULL)->floatval;
      if (divisor == 0) {
        return error_obj(ERR_DIVISION_BY_ZERO);
      }
      return float_obj(obj->floatval / divisor);
    }
    default:
      printf("method_id %d not implemented!\n", method_id);
      return obj;
  }
}

obj_t *float_add(obj_t *obj, obj_method_args_t *args) {
  return float_math(obj, args, METHOD_ADD);
}

obj_t *float_sub(obj_t *obj, obj_method_args_t *args) {
  return float_math(obj, args, METHOD_SUB);
}

obj_t *float_mul(obj_t *obj, obj_method_args_t *args) {
  return float_math(obj, args, METHOD_MUL);
}

obj_t *float_div(obj_t *obj, obj_method_args_t *args) {
  return float_math(obj, args, METHOD_DIV);
}

obj_t *float_eq(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_FLOAT:
      return boolean_obj((obj->floatval == arg->floatval) ? True : False);
    case TYPE_INT:
      return boolean_obj((obj->floatval == arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
             type_names[obj->type], type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *float_ne(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_FLOAT && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           type_names[obj->type], type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *eq = float_eq(obj, args);
  return boolean_obj(eq->boolval == True ? False : True);
}

obj_t *float_lt(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_FLOAT:
      return boolean_obj((obj->floatval < arg->floatval) ? True : False);
    case TYPE_INT:
      return boolean_obj((obj->floatval < arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
            type_names[obj->type], type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *float_gt(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  switch(arg->type) {
    case TYPE_FLOAT:
      return boolean_obj((obj->floatval > arg->floatval) ? True : False);
    case TYPE_INT:
      return boolean_obj((obj->floatval > arg->intval) ? True : False);
    default:
      printf("Cannot compare for equality between %s and %s.\n",
            type_names[obj->type], type_names[arg->type]);
      return boolean_obj(False);
  }
}

obj_t *float_le(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_FLOAT && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           type_names[obj->type], type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *gt = float_gt(obj, args);
  return boolean_obj((gt->boolval == True) ? False : True);
}

obj_t *float_ge(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;
  if (arg->type != TYPE_FLOAT && arg->type != TYPE_INT) {
    printf("Cannot compare for equality between %s and %s.\n",
           type_names[obj->type], type_names[arg->type]);
    return boolean_obj(False);
  }

  obj_t *lt = float_lt(obj, args);
  return boolean_obj((lt->boolval == True) ? False : True);
}

obj_t *float_as(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  if (arg->type == TYPE_FLOAT) {
    return obj;
  }

  if (arg->type == TYPE_INT) {
    return int_obj((int) obj->floatval);
  }

  if (arg->type == TYPE_STRING) {
    return float_to_string(obj, NULL);
  }

  if (arg->type == TYPE_BOOLEAN) {
    return boolean_obj(obj->floatval ? True : False);
  }

  printf("Cannot cast %s to %s.\n",
          type_names[obj->type], type_names[arg->type]);
  return nil_obj();
}

static_method get_float_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return float_hash;
    case METHOD_COPY: return float_copy;
    case METHOD_TO_INT: return float_to_int;
    case METHOD_TO_STRING: return float_to_string;
    case METHOD_TO_FLOAT: return float_to_float;
    case METHOD_TO_BYTE: return float_to_byte;
    case METHOD_ABS: return float_abs;
    case METHOD_NEG: return float_neg;
    case METHOD_EQ: return float_eq;
    case METHOD_NE: return float_ne;
    case METHOD_LT: return float_lt;
    case METHOD_GT: return float_gt;
    case METHOD_LE: return float_le;
    case METHOD_GE: return float_ge;
    case METHOD_ADD: return float_add;
    case METHOD_SUB: return float_sub;
    case METHOD_MUL: return float_mul;
    case METHOD_DIV: return float_div;
    case METHOD_CAST: return float_as;
    default: return NULL;
  }
}

