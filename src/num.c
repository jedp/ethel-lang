#include "../inc/obj.h"
#include "../inc/num.h"

boolean is_numeric(obj_t *obj) {
  return obj->type == TYPE_INT || obj->type == TYPE_FLOAT;
}

obj_t *num_sub(obj_t *obj, obj_t *other) {
  if (!is_numeric(obj) || !is_numeric(other)) return error_obj(ERR_EVAL_TYPE_ERROR);

  if ((obj->type == TYPE_INT) && (other->type == TYPE_INT)) {
    return int_obj(obj->intval - other->intval);
  }

  if ((obj->type == TYPE_INT) && (other->type == TYPE_FLOAT)) {
    return float_obj(obj->intval - other->floatval);
  }

  if ((obj->type == TYPE_FLOAT) && (other->type == TYPE_INT)) {
    return float_obj(obj->floatval - other->intval);
  }

  if ((obj->type == TYPE_FLOAT) && (other->type == TYPE_FLOAT)) {
    return float_obj(obj->floatval - other->floatval);
  }

  return error_obj(ERR_EVAL_TYPE_ERROR);
}

obj_t *num_mul(obj_t *obj, obj_t *other) {
  if (!is_numeric(obj) || !is_numeric(other)) return error_obj(ERR_EVAL_TYPE_ERROR);

  if ((obj->type == TYPE_INT) && (other->type == TYPE_INT)) {
    return int_obj(obj->intval * other->intval);
  }

  if ((obj->type == TYPE_INT) && (other->type == TYPE_FLOAT)) {
    return float_obj(obj->intval * other->floatval);
  }

  if ((obj->type == TYPE_FLOAT) && (other->type == TYPE_INT)) {
    return float_obj(obj->floatval * other->intval);
  }

  if ((obj->type == TYPE_FLOAT) && (other->type == TYPE_FLOAT)) {
    return float_obj(obj->floatval * other->floatval);
  }

  return error_obj(ERR_EVAL_TYPE_ERROR);
}

obj_t *num_div(obj_t *obj, obj_t *other) {
  if (!is_numeric(obj) || !is_numeric(other)) return error_obj(ERR_EVAL_TYPE_ERROR);

  if ((other->type == TYPE_INT && other->intval == 0) ||
      (other->type == TYPE_FLOAT && other->floatval == 0)) {
    return error_obj(ERR_DIVISION_BY_ZERO);
  }

  if ((obj->type == TYPE_INT) && (other->type == TYPE_INT)) {
    return int_obj(obj->intval / other->intval);
  }

  if ((obj->type == TYPE_INT) && (other->type == TYPE_FLOAT)) {
    return float_obj(obj->intval / other->floatval);
  }

  if ((obj->type == TYPE_FLOAT) && (other->type == TYPE_INT)) {
    return float_obj(obj->floatval / other->intval);
  }

  if ((obj->type == TYPE_FLOAT) && (other->type == TYPE_FLOAT)) {
    return float_obj(obj->floatval / other->floatval);
  }

  return error_obj(ERR_EVAL_TYPE_ERROR);
}

obj_t *num_mod(obj_t *obj, obj_t *other) {
  if (!is_numeric(obj) || !is_numeric(other)) return error_obj(ERR_EVAL_TYPE_ERROR);

  if ((other->type == TYPE_INT && other->intval == 0) ||
      (other->type == TYPE_FLOAT && other->floatval == 0)) {
    return error_obj(ERR_DIVISION_BY_ZERO);
  }

  if ((obj->type == TYPE_INT) && (other->type == TYPE_INT)) {
    return int_obj(obj->intval % other->intval);
  }

  if ((obj->type == TYPE_INT) && (other->type == TYPE_FLOAT)) {
    return int_obj(obj->intval % (int) other->floatval);
  }

  if ((obj->type == TYPE_FLOAT) && (other->type == TYPE_INT)) {
    return int_obj((int) obj->floatval % other->intval);
  }

  if ((obj->type == TYPE_FLOAT) && (other->type == TYPE_FLOAT)) {
    return int_obj((int) obj->floatval % (int) other->floatval);
  }

  return error_obj(ERR_EVAL_TYPE_ERROR);
}

obj_t *num_eq(obj_t *obj, obj_t *other) {
  if (!is_numeric(obj) || !is_numeric(other)) return error_obj(ERR_EVAL_TYPE_ERROR);

  float a = obj->type == TYPE_INT ? obj->intval : obj->floatval;
  float b = other->type == TYPE_INT ? other->intval : other->floatval;
  return boolean_obj(a == b);
}

obj_t *num_lt(obj_t *obj, obj_t *other) {
  if (!is_numeric(obj) || !is_numeric(other)) return error_obj(ERR_EVAL_TYPE_ERROR);

  float a = obj->type == TYPE_INT ? obj->intval : obj->floatval;
  float b = other->type == TYPE_INT ? other->intval : other->floatval;
  return boolean_obj(a < b);
}

obj_t *num_le(obj_t *obj, obj_t *other) {
  if (!is_numeric(obj) || !is_numeric(other)) return error_obj(ERR_EVAL_TYPE_ERROR);

  float a = obj->type == TYPE_INT ? obj->intval : obj->floatval;
  float b = other->type == TYPE_INT ? other->intval : other->floatval;
  return boolean_obj(a <= b);
}

obj_t *num_gt(obj_t *obj, obj_t *other) {
  if (!is_numeric(obj) || !is_numeric(other)) return error_obj(ERR_EVAL_TYPE_ERROR);

  float a = obj->type == TYPE_INT ? obj->intval : obj->floatval;
  float b = other->type == TYPE_INT ? other->intval : other->floatval;
  return boolean_obj(a > b);
}

obj_t *num_ge(obj_t *obj, obj_t *other) {
  if (!is_numeric(obj) || !is_numeric(other)) return error_obj(ERR_EVAL_TYPE_ERROR);

  float a = obj->type == TYPE_INT ? obj->intval : obj->floatval;
  float b = other->type == TYPE_INT ? other->intval : other->floatval;
  return boolean_obj(a >= b);
}

obj_t *num_ne(obj_t *obj, obj_t *other) {
  if (!is_numeric(obj) || !is_numeric(other)) return error_obj(ERR_EVAL_TYPE_ERROR);

  float a = obj->type == TYPE_INT ? obj->intval : obj->floatval;
  float b = other->type == TYPE_INT ? other->intval : other->floatval;
  return boolean_obj(a != b);
}

