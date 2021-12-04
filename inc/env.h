#ifndef __ENV_H
#define __ENV_H

#include "err.h"
#include "obj.h"

#define ENV_MAX_STACK_DEPTH 50

typedef struct __attribute__((__packed__)) Symbol {
    obj_t *name_obj;
    uint16_t flags;
    /* Used to track how many times this scope is on the stack. */
    /* When refcount is 0, all symbols in the scope can be deleted. */
    uint16_t refcount;
    struct Obj *obj;
    struct Symbol *prev;
    struct Symbol *next;
} env_sym_t;

/* A stack of symbol tables in scope. */
typedef struct Env {
    int top;
    env_sym_t *symbols[ENV_MAX_STACK_DEPTH];
} env_t;

error_t env_init(env_t *env);
error_t push_scope(env_t *env, env_sym_t *scope);
error_t enter_scope(env_t *env);
error_t leave_scope(env_t *env);
error_t put_env(env_t *env, obj_t *name_string_obj, const obj_t *obj, const uint16_t flags);
error_t put_env_shadow(env_t *env, obj_t *name_string_obj, const obj_t *obj, const uint16_t flags);
error_t del_env(env_t *env, obj_t *name_string_obj);
obj_t *get_env(env_t *env, obj_t *name_string_obj);

#endif

