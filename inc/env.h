#ifndef __ENV_H
#define __ENV_H

#include "err.h"
#include "obj.h"

#define ENV_MAX_SCOPES 16

typedef struct Symbol {
    char* name;
    uint8_t flags;
    struct Obj *obj;
    struct Symbol *next;
} env_sym_t;

/* A stack of symbol tables in scope. */
typedef struct Env {
    int top;
    env_sym_t symbols[ENV_MAX_SCOPES];
} env_t;

error_t env_init(env_t *env);
error_t push_scope(env_t *env);
error_t pop_scope(env_t *env);
error_t put_env(env_t *env, const char* name, const obj_t *obj, const uint8_t flags);
obj_t *get_env(env_t *env, const char* name);

#endif

