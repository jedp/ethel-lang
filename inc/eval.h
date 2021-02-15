#ifndef _EVAL_H
#define _EVAL_H

#include <inttypes.h>
#include "ast.h"

typedef enum {
    RES_NONE = 0,
    RES_INT = 1,
    RES_FLOAT = 2,
    RES_STRING = 3,
} result_t;

typedef struct Result {
    uint8_t err;
    uint8_t type;
    union {
        int intval;
        float floatval;
        char *stringval;
    };
} eval_result_t;

eval_result_t *eval(char* input);

#endif

