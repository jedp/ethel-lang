#include <stdio.h>
#include "../inc/mem.h"
#include "../inc/def.h"
#include "../inc/str.h"
#include "../inc/env.h"

env_sym_t *new_sym(bytearray_t *name, obj_t *obj, uint16_t flags) {
  env_sym_t *sym = mem_alloc(sizeof(env_sym_t));
  sym->name = mem_alloc(sizeof(bytearray_t));
  sym->name = bytearray_clone(name);
  sym->flags = flags;
  sym->refcount = 0;
  sym->obj = obj;
  sym->prev = NULL;
  sym->next = NULL; 

  return sym;
}

env_sym_t *empty_sym() {
  return new_sym(c_str_to_bytearray(""), NULL, F_NONE);
}

error_t push_scope(env_t *env, env_sym_t *scope) {
  env->top += 1;
  if (env->top == ENV_MAX_STACK_DEPTH) {
    return ERR_ENV_MAX_DEPTH_EXCEEDED;
  }

  env->symbols[env->top] = *scope;
  scope->refcount += 1;
  return ERR_NO_ERROR;
}

error_t enter_scope(env_t *env) {
  return push_scope(env, empty_sym());
}

error_t leave_scope(env_t *env) {
  env_sym_t *next = env->symbols[env->top].next;
  // TODO buggy
  // Decrement refcount when popping scope.
  // Once refcount is 0, we know it's not under something else's scope.
  if (next != NULL && --next->refcount < 1) {
    // Delete any symbols at this level.
    // Don't delete the root node.
    while (next != NULL) {
      env_sym_t *temp = next;
      next = next->next;
      mem_free(temp->name);
      mem_free(temp);
      temp = NULL;
    }
  }
  env->top -= 1;

  return ERR_NO_ERROR;
}

static env_sym_t *find_sym(env_t *env, bytearray_t *name, boolean recursive) {
  if (env->top < 0) {
    return NULL;
  }

  // Search back through the scopes to find the name.
  for (int i = env->top; i >= 0; --i) {
    // Start at the node the root points to.
    env_sym_t *node = env->symbols[i].next;
    while (node != NULL) {
      if (bytearray_eq(name, node->name)) {
        return node;
      }
      node = node->next;
    }

    if (!recursive) return NULL;
  }

  return NULL;
}

error_t _put_env(env_t *env,
                 bytearray_t *name,
                 const obj_t *obj,
                 const uint16_t flags,
                 boolean can_shadow) {
  if (env->top < 0) {
    return ERR_ENV_NO_SCOPE;
  }

  env_sym_t *found = find_sym(env, name, !can_shadow);
  // Already exists in scopes we can access.
  if (found != NULL) {
    if (!(found->flags & F_VAR)) {
      return ERR_ENV_SYMBOL_REDEFINED;
    }

    // Preserve flags, copying new object.
    found->obj = (obj_t *) obj;
    return ERR_NO_ERROR;
  }

  // Not found. Put it in the current scope.
  env_sym_t *top = &(env->symbols[env->top]);
  env_sym_t *new = new_sym(name, (obj_t *) obj, flags);
  new->next = top->next;
  if (top->next != NULL) top->next->prev = new;
  new->prev = top;
  top->next = new;

  return ERR_NO_ERROR;
}

error_t put_env(env_t *env, bytearray_t *name, const obj_t *obj, const uint16_t flags) {
  return _put_env(env, name, obj, flags, False);
}

error_t put_env_shadow(env_t *env, bytearray_t *name, const obj_t *obj, const uint16_t flags) {
  return _put_env(env, name, obj, flags, True);
}

error_t del_env(env_t *env, bytearray_t *name) {
  env_sym_t *sym = find_sym(env, name, True);
  if (sym == NULL) return ERR_ENV_SYMBOL_UNDEFINED;

  env_sym_t *prev = sym->prev;
  if (sym->next != NULL) sym->next->prev = prev;
  prev->next = sym->next;
  mem_free(sym);

  return ERR_NO_ERROR;
}

obj_t *get_env(env_t *env, bytearray_t *name) {
  env_sym_t *sym = find_sym(env, name, True);
  if (sym == NULL) return undef_obj();

  return sym->obj;
}

error_t env_init(env_t *env) {
  for (int i = 0; i < ENV_MAX_STACK_DEPTH; ++i) {
    env->symbols[i] = *empty_sym();
  }

  // An outer new_scope() is required.
  env->top = -1;

  return ERR_NO_ERROR;
}

