#ifndef _EVAL_H
#define _EVAL_H

#include <inttypes.h>
#include "obj.h"
#include "ast.h"
#include "env.h"

typedef struct Result {
    uint8_t err;
    struct Obj *obj;
} eval_result_t;

eval_result_t *eval(env_t *env, char* input);

#endif

