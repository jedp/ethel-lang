#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "../inc/def.h"
#include "../inc/env.h"

env_sym_t *new_sym(const char* name, obj_t *obj, uint8_t flags) {
  env_sym_t *sym = malloc(sizeof(env_sym_t));
  char* sym_name = malloc(strlen(name) + 1);
  strcpy(sym_name, name);
  sym->name = sym_name;
  sym->flags = flags;
  sym->obj = obj;
  sym->prev = NULL;
  sym->next = NULL; 

  return sym;
}

env_sym_t *empty_sym() {
  return new_sym("", NULL, F_NONE);
}

error_t push_scope(env_t *env) {
  env->top += 1;
  if (env->top == ENV_MAX_SCOPES) {
    return ENV_MAX_DEPTH_EXCEEDED;
  }

  env->symbols[env->top] = *empty_sym();

  return NO_ERROR;
}

error_t pop_scope(env_t *env) {
  env->symbols[env->top] = *empty_sym();
  env->top -= 1;

  return NO_ERROR;
}

env_sym_t *find_sym(env_t *env, const char *name) {
  if (env->top < 0) {
    return NULL;
  }

  // Search back through the scopes to find the name.
  for (int i = env->top; i >= 0; --i) {
    env_sym_t *last = &(env->symbols[i]);
    while (last->next != NULL) {
      if (!strcmp(name, last->name)) {
        return last;
      }
      last = last->next;
    }

    // Check the last one.
    if (!strcmp(name, last->name)) {
      return last;
    }
  }

  return NULL;
}

error_t put_env(env_t *env, const char* name, const obj_t *obj, const uint8_t flags) {
  if (env->top < 0) {
    return ENV_NO_SCOPE;
  }

  env_sym_t *found = find_sym(env, name);
  // Already exists in scopes we can access.
  if (found != NULL) {
    if (!(found->flags & F_VAR)) {
      return ENV_SYMBOL_REDEFINED;
    }

    // Preserve flags, copying new object.
    found->obj = (obj_t *) obj;
    return NO_ERROR;
  }

  // Not found. Put it in the current scope.

  env_sym_t *top = &(env->symbols[env->top]);
  env_sym_t *new = new_sym(name, (obj_t *) obj, flags);
  new->next = top->next;
  if (top->next != NULL) top->next->prev = new;
  new->prev = top;
  top->next = new;

  return NO_ERROR;
}

error_t del_env(env_t *env, const char* name) {
  env_sym_t *sym = find_sym(env, name);
  if (sym == NULL) return ENV_SYMBOL_UNDEFINED;

  env_sym_t *prev = sym->prev;
  if (sym->next != NULL) sym->next->prev = prev;
  prev->next = sym->next;
  free(sym);

  return NO_ERROR;
}

obj_t *get_env(env_t *env, const char* name) {
  env_sym_t *sym = find_sym(env, name);
  if (sym == NULL) return undef_obj();

  return sym->obj;
}

error_t env_init(env_t *env) {
  for (int i = 0; i < ENV_MAX_SCOPES; ++i) {
    env->symbols[i] = *empty_sym();
  }

  // An outer push_scope() is required.
  env->top = -1;

  return NO_ERROR;
}

