#ifndef __ENV_H
#define __ENV_H

#include "err.h"
#include "obj.h"

#define ENV_MAX_STACK_DEPTH 50

typedef struct Env {
    gc_header_t hdr;
    struct Env *parent;
    obj_t *vars;
} env_t;

typedef struct InterpState {
    int top;
    env_t *env;
    env_t *ret_stack[ENV_MAX_STACK_DEPTH];
} interp_t;

error_t interp_init(interp_t *interp);

env_t *new_env(void);

error_t push_scope(interp_t *interp, env_t *scope);

error_t enter_scope(interp_t *interp);

error_t leave_scope(interp_t *interp);

/*
 * Associate a name with an obj for lookup, and put them both in the current
 * environment scope. Objects may shadow other objects in higher scopes.
 *
 * They will be protected from GC as long as they are in scope.
 */
error_t put_env(interp_t *interp, bytearray_t *name_obj, obj_t *obj);

/*
 * Plant a GC root for an object in the current scope.
 *
 * These objects are stored anonymously and cannot be retrieved. They can be
 * cleaned up via GC after the scope exits.
 */
error_t put_env_gc_root(interp_t *interp, gc_header_t *hdr);

/*
 * Delete the named object from the current scope.
 */
error_t del_env(interp_t *interp, bytearray_t *name_obj);

/*
 * Get an obj_t object with the given name from the current scope.
 */
obj_t *get_env(interp_t *interp, bytearray_t *name_obj);

void show_env(interp_t *interp);

#endif
