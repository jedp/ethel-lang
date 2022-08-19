#include <assert.h>
#include <stdio.h>
#include "../inc/type.h"
#include "../inc/dict.h"
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/env.h"

env_t *new_env(void) {
    env_t *env = (env_t *) alloc_type(INTERP_ENV, F_NONE);
    env->parent = NULL;
    env->vars = dict_obj();
    return env;
}

error_t push_scope(interp_t *interp, env_t *scope) {
    interp->top += 1;
    if (interp->top == ENV_MAX_STACK_DEPTH) {
        return ERR_ENV_MAX_DEPTH_EXCEEDED;
    }
    interp->ret_stack[interp->top] = scope;
    interp->env = interp->ret_stack[interp->top];
    return ERR_NO_ERROR;
}

error_t enter_scope(interp_t *interp) {
    env_t *env = new_env();
    env->parent = interp->env;
    return push_scope(interp, env);
}

error_t leave_scope(interp_t *interp) {
    if (interp->top == 0) {
        return ERR_UNDERFLOW_ERROR;
    }

    interp->ret_stack[interp->top] = NULL;
    interp->top -= 1;
    interp->env = interp->ret_stack[interp->top];

    return ERR_NO_ERROR;
}

static error_t put_env_internal(interp_t *interp,
                                bytearray_t *name_obj,
                                gc_header_t *obj) {
    env_t *env = interp->env;
    obj_t *found;

    // New declaration in this scope? (Can shadow.)
    if (obj->flags & F_ENV_DECLARATION) {
        found = dict_get(env->vars, string_obj(name_obj));
        if (found->hdr.type != TYPE_NIL) {
            return ERR_ENV_SYMBOL_REDEFINED;
        }
        // Strip off the declaration flag. Don't need to preserve that.
        obj->flags &= ~F_ENV_DECLARATION;
        return dict_put(env->vars, string_obj(name_obj), (obj_t *) obj);
    }

    // (Re-)assignment in this or a higher scope?
    while (env != NULL) {
        found = dict_get(env->vars, string_obj(name_obj));

        if (found->hdr.type != TYPE_NIL) {
            if (!(found->hdr.flags & F_ENV_MUTABLE) &&
                !(found->hdr.flags & F_ENV_OVERWRITE)) {
                return ERR_ENV_SYMBOL_REDEFINED;
            }

            // Mutate, preserving original flags.
            obj->flags = found->hdr.flags;
            return dict_put(env->vars, string_obj(name_obj), (obj_t *) obj);
        }

        // Keep looking in the parent env.
        env = env->parent;
    }

    // First-time declaration of loop variable?
    if (obj->flags & F_ENV_OVERWRITE) {
        return dict_put(interp->env->vars, string_obj(name_obj), (obj_t *) obj);
    }

    return ERR_ENV_SYMBOL_UNDEFINED;
}

error_t put_env(interp_t *interp, bytearray_t *name_obj, gc_header_t *hdr) {
    return put_env_internal(interp, name_obj, hdr);
}

error_t put_env_gc_root(interp_t *interp, gc_header_t *hdr) {
    return put_env_internal(interp, c_str_to_bytearray("<gc-root>"), hdr);
}

error_t del_env(interp_t *interp, bytearray_t *name_obj) {
    dict_remove(interp->env->vars, string_obj(name_obj));
    return ERR_NO_ERROR;
}

obj_t *get_env(interp_t *interp, bytearray_t *name_obj) {
    obj_t *found;
    env_t *env = interp->env;
    while (env != NULL) {
        found = dict_get(env->vars, string_obj(name_obj));
        if (found->hdr.type != TYPE_NIL) {
            return found;
        }
        assert(env != env->parent);
        env = env->parent;
    }

    return undef_obj();
}

error_t interp_init(interp_t *interp) {
    env_t *env = new_env();

    for (int i = 0; i < ENV_MAX_STACK_DEPTH; ++i) {
        interp->ret_stack[i] = NULL;
    }
    interp->ret_stack[0] = env;
    interp->top = 0;
    interp->env = env;

    return ERR_NO_ERROR;
}

void show_env(interp_t *interp) {
    printf("interp: top %d, env %p\n", interp->top, interp->env);
    for (int i = interp->top; i >= 0; --i) {
        printf("ret%d:\n env %p\n parent %p\n", i,
               interp->ret_stack[i],
               interp->ret_stack[i]->parent);
    }
}
