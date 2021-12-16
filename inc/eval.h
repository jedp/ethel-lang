#ifndef _EVAL_H
#define _EVAL_H

#include <inttypes.h>
#include "obj.h"
#include "ast.h"
#include "env.h"

typedef struct Result {
  gc_header_t hdr;
  obj_t* obj;
  uint16_t depth;
  uint16_t err;
} eval_result_t;

void eval(env_t *env, const char* input, eval_result_t* result);

#endif

