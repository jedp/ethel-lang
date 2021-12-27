#include <assert.h>
#include <stdio.h>
#include "../inc/type.h"
#include "../inc/mem.h"
#include "../inc/def.h"
#include "../inc/str.h"
#include "../inc/env.h"

env_sym_t *new_sym(bytearray_t *name_obj, gc_header_t *hdr, flags_t flags) {
  env_sym_t *sym = (env_sym_t*) alloc_type(ENV_SYM, flags);
  sym->name_obj = bytearray_clone(name_obj);
  sym->obj = hdr;
  sym->prev = NULL;
  sym->next = NULL;

  return sym;
}

error_t push_scope(env_t *env, env_sym_t *scope) {
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
  env->symbols[env->top] = NULL;
  env->top -= 1;

  return ERR_NO_ERROR;
}

static env_sym_t *find_sym_by_name(env_t *env, bytearray_t *sym_name, boolean recursive) {
  if (env->top < 0) return NULL;

  // NULL is a valid name for planting gc roots.
  if (sym_name == NULL) return NULL;

  bytearray_t *name = sym_name;
  // Search back through the scopes to find the name.
  for (int i = env->top; i >= 0; --i) {
    // Start at the node the root points to.
    env_sym_t *node = env->symbols[i];
    while (node != NULL) {
      if (node->name_obj != NULL && bytearray_eq(name, node->name_obj)) {
        return node;
      }
      node = node->next;
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

  env_sym_t* found = find_sym_by_name(env, name_obj, !can_shadow);
  // Already exists in scopes we can access.
  if (found != NULL) {
    if (!(flags & F_ENV_OVERWRITE) && !(FLAGS(found) & F_ENV_VAR)) {
      return ERR_ENV_SYMBOL_REDEFINED;
    }

    found->obj = (gc_header_t*) obj;
    return ERR_NO_ERROR;
  }

  // Not found. Put it in the current scope.
  env_sym_t *top = env->symbols[env->top];
  env_sym_t *new = new_sym(name_obj, (gc_header_t*) obj, flags);

  // First thing in this scope?
  if (top == NULL) {
    env->symbols[env->top]= new;
    return ERR_NO_ERROR;
  }

  // Insert into list of other things at this scope.
  assert(top->prev == NULL);
  new->prev = NULL;
  new->next = top;
  top->prev = new;
  env->symbols[env->top] = new;

  return ERR_NO_ERROR;
}

error_t put_env(env_t *env, bytearray_t *name_obj, const obj_t *obj, const flags_t flags) {
  return _put_env(env, name_obj, (gc_header_t*) obj, flags, False);
}

error_t put_env_shadow(env_t *env, bytearray_t *name_obj, const obj_t *obj, const flags_t flags) {
  return _put_env(env, name_obj, (gc_header_t*) obj, flags, True);
}

error_t put_env_gc_root(env_t *env, const gc_header_t *hdr) {
  return _put_env(env, NULL, hdr, F_NONE, False);
}

error_t del_env(env_t *env, bytearray_t *name_obj) {
  env_sym_t *sym = find_sym_by_name(env, name_obj, True);
  if (sym == NULL) return ERR_ENV_SYMBOL_UNDEFINED;

  env_sym_t *prev = sym->prev;
  if (prev == NULL && sym->next == NULL) {
    // Only element in list.
    env->symbols[env->top] = NULL;
    return ERR_NO_ERROR;
  }

  if (prev == NULL) {
    // First element in list.
    env->symbols[env->top] = sym->next;
    env->symbols[env->top]->prev = NULL;
    sym = NULL;
    return ERR_NO_ERROR;
  }

  if (sym->next == NULL) {
    // Last element in list with preceding elements.
    prev->next = NULL;
    sym = NULL;
    return ERR_NO_ERROR;
  }

  // Somewhere in the middle of the list.
  prev->next = sym->next;
  sym = NULL;
  return ERR_NO_ERROR;
}

obj_t *get_env(env_t *env, bytearray_t *name_obj) {
  env_sym_t *sym = find_sym_by_name(env, name_obj, True);
  if (sym == NULL) return undef_obj();

  return (obj_t*) sym->obj;
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
  env_sym_t *sym;

  printf("Env:\n");
  for (int i = env->top; i >= 0; --i) {
    printf("[Level %d]\n", i);
    sym = env->symbols[i];
    while (sym != NULL) {
      if (sym->name_obj == NULL) {
        printf("  <hidden> %s\n", type_names[TYPEOF(sym->obj)]);
      } else {
        printf("  '%s' %s\n", bytearray_to_c_str(sym->name_obj), type_names[TYPEOF(sym->obj)]);
      }
      sym = sym->next;
    }
  }
  printf("----\n");
}
