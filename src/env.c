#include <assert.h>
#include <stdio.h>
#include "../inc/type.h"
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/env.h"

static env_sym_t *new_sym(bytearray_t *name_obj, gc_header_t *obj, flags_t flags) {
    env_sym_t *sym = (env_sym_t *) alloc_type(ENV_SYM, flags);
    sym->name_obj = bytearray_clone(name_obj);
    sym->obj = obj;

    return sym;
}

static env_sym_elem_t *new_sym_elem(env_sym_t *sym) {
    env_sym_elem_t *elem = (env_sym_elem_t *) alloc_type(ENV_SYM_ELEM, F_NONE);
    elem->sym = (env_sym_t *) sym;
    elem->prev = NULL;
    elem->next = NULL;

    return elem;
}

error_t push_scope(env_t *env, env_sym_elem_t *scope) {
    env->top += 1;
    if (env->top == ENV_MAX_STACK_DEPTH) {
        return ERR_ENV_MAX_DEPTH_EXCEEDED;
    }

    env->symbols[env->top] = scope;
    return ERR_NO_ERROR;
}

error_t enter_scope(env_t *env) {
    return push_scope(env, NULL);
}

error_t leave_scope(env_t *env) {
    env->symbols[env->top] = new_sym_elem(NULL);
    env->top -= 1;

    return ERR_NO_ERROR;
}

static env_sym_t *find_sym_by_name(env_t *env, bytearray_t *sym_name, boolean recursive) {
    if (env->top < 0) return NULL;

    // NULL is a valid name for planting gc roots.
    if (sym_name == NULL) return NULL;

    // Search back through the scopes to find the name.
    for (int i = env->top; i >= 0; --i) {
        // Start at the node the root points to.
        env_sym_elem_t *elem = env->symbols[i];
        while (elem != NULL) {
            if (elem->sym != NULL
                && elem->sym->name_obj != NULL
                && bytearray_eq(sym_name, elem->sym->name_obj)) {
                return elem->sym;
            }
            elem = elem->next;
        }

        if (!recursive) return NULL;
    }

    return NULL;
}

error_t _put_env(env_t *env,
                 bytearray_t *name_obj,
                 const gc_header_t *obj,
                 const flags_t flags,
                 boolean can_shadow) {
    assert(env->top >= 0);

    env_sym_t *found = find_sym_by_name(env, name_obj, !can_shadow);
    // Already exists in scopes we can access.
    if (found != NULL) {
        if (!(flags & F_ENV_OVERWRITE) && !(FLAGS(found) & F_ENV_VAR)) {
            return ERR_ENV_SYMBOL_REDEFINED;
        }

        found->obj = (gc_header_t *) obj;
        return ERR_NO_ERROR;
    }

    // Not found. Put it in the current scope.
    env_sym_t *new = new_sym(name_obj, (gc_header_t *) obj, flags);

    env_sym_elem_t *top_elem = env->symbols[env->top];
    env_sym_elem_t *new_elem = new_sym_elem(new);

    // First thing in this scope?
    if (top_elem == NULL) {
        env->symbols[env->top] = new_elem;
        return ERR_NO_ERROR;
    }

    // Insert into list of other things at this scope.
    assert(top_elem->prev == NULL);
    new_elem->prev = NULL;
    new_elem->next = top_elem;
    top_elem->prev = new_elem;
    env->symbols[env->top] = new_elem;

    return ERR_NO_ERROR;
}

error_t put_env(env_t *env, bytearray_t *name_obj, const obj_t *obj, const flags_t flags) {
    return _put_env(env, name_obj, (gc_header_t *) obj, flags, False);
}

error_t put_env_shadow(env_t *env, bytearray_t *name_obj, const obj_t *obj, const flags_t flags) {
    return _put_env(env, name_obj, (gc_header_t *) obj, flags, True);
}

error_t put_env_gc_root(env_t *env, const gc_header_t *hdr) {
    return _put_env(env, NULL, hdr, F_NONE, False);
}

error_t del_env(env_t *env, bytearray_t *name_obj) {
    env_sym_elem_t *sym_elem = env->symbols[env->top];

    while (sym_elem != NULL) {
        if (sym_elem->sym != NULL
            && sym_elem->sym->name_obj != NULL
            && bytearray_eq(name_obj, sym_elem->sym->name_obj)) {
            sym_elem->sym = NULL;

            if (sym_elem->prev != NULL) sym_elem->prev = sym_elem->next;
            if (sym_elem->next != NULL) sym_elem->next = sym_elem->prev;
            sym_elem = NULL;

            return ERR_NO_ERROR;
        }
        sym_elem = sym_elem->next;
    }

    return ERR_ENV_SYMBOL_UNDEFINED;
}

obj_t *get_env(env_t *env, bytearray_t *name_obj) {
    env_sym_t *sym = find_sym_by_name(env, name_obj, True);
    if (sym == NULL) return undef_obj();

    return (obj_t *) sym->obj;
}

error_t env_init(env_t *env) {
    // Intialize all pointers.
    for (int i = 0; i < ENV_MAX_STACK_DEPTH; ++i) {
        env->symbols[i] = NULL;
    }

    // An outer new_scope() is required.
    env->top = -1;

    return ERR_NO_ERROR;
}

void show_env(env_t *env) {
    env_sym_elem_t *sym_elem;

    printf("Env:\n");
    for (int i = env->top; i >= 0; --i) {
        printf("[Level %d]\n", i);
        sym_elem = env->symbols[i];
        while (sym_elem != NULL) {
            if (sym_elem->sym != NULL) {
                if (sym_elem->sym->name_obj == NULL) {
                    printf("  <hidden> %s\n", type_names[TYPEOF(sym_elem->sym->obj)]);
                } else {
                    printf("  '%s' %s\n", bytearray_to_c_str(sym_elem->sym->name_obj),
                           type_names[TYPEOF(sym_elem->sym->obj)]);
                }
            }
            sym_elem = sym_elem->next;
        }
    }
    printf("----\n");
}
