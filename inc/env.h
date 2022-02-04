#ifndef __ENV_H
#define __ENV_H

#include "err.h"
#include "obj.h"

#define ENV_MAX_STACK_DEPTH 50

typedef struct Symbol {
    gc_header_t hdr;
    bytearray_t *name_obj;
    gc_header_t *obj;
} env_sym_t;

typedef struct SymbolElem {
    gc_header_t hdr;
    env_sym_t *sym;
    struct SymbolElem *prev;
    struct SymbolElem *next;
} env_sym_elem_t;

/*
 * A stack of symbol tables in scope.
 *
 * This structure will be statically allocated.
 */
typedef struct Env {
    int top;
    env_sym_elem_t *symbols[ENV_MAX_STACK_DEPTH];
} env_t;

error_t env_init(env_t *env);

error_t push_scope(env_t *env, env_sym_elem_t *scope);

error_t enter_scope(env_t *env);

error_t leave_scope(env_t *env);

/*
 * Associate a name with an obj for lookup, and put them both in the current
 * environment scope. Objects may not shadow other objects in higher scopes.
 *
 * They will be protected from GC as long as they are in scope.
 */
error_t put_env(env_t *env, bytearray_t *name_obj, const obj_t *obj, const flags_t flags);

/*
 * Associate a name with an obj for lookup, and put them both in the current
 * environment scope. The obj is allowed to shadow another object in a higher
 * scope.
 *
 * They will be protected from GC as long as they are in scope.
 */
error_t put_env_shadow(env_t *env, bytearray_t *name_obj, const obj_t *obj, const flags_t flags);

/*
 * Plant a GC root for an object in the current scope.
 *
 * These objects are stored anonymously and cannot be retrieved. They can be
 * cleaned up via GC after the scope exits.
 */
error_t put_env_gc_root(env_t *env, const gc_header_t *hdr);

/*
 * Delete the named object from the current scope.
 */
error_t del_env(env_t *env, bytearray_t *name_obj);

/*
 * Get an obj_t object with the given name from the current scope.
 */
obj_t *get_env(env_t *env, bytearray_t *name_obj);

void show_env(env_t *env);

#endif
