#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "../inc/env.h"

env_sym_t *new_sym(const char* name) {
  env_sym_t *sym = malloc(sizeof(env_sym_t));
  char* sym_name = malloc(strlen(name) + 1);
  strcpy(sym_name, name);
  sym->name = sym_name;
  sym->next = NULL; 

  return sym;
}

env_sym_t *empty_sym() {
  return new_sym("");
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

error_t put_env(env_t *env, const char* name, const obj_t *obj) {
  if (env->top < 0) {
    return ENV_NO_SCOPE;
  }

  env_sym_t *last = &(env->symbols[env->top]);
  while (last->next != NULL) {
    if (!strcmp(name, last->name)) {
      return ENV_SYMBOL_REDEFINED;
    }
    last = last->next;
  }

  // Check the last one.
  if (!strcmp(name, last->name)) {
    return ENV_SYMBOL_REDEFINED;
  }

  env_sym_t *sym = new_sym(name);
  sym->obj = (obj_t*) obj;
  last->next = sym;

  return NO_ERROR;
}

obj_t *get_env(env_t *env, const char* name) {
  if (env->top < 0) {
    return undef_obj();
  }

  // Search back through the scopes to find the name.
  for (int i = env->top; i >= 0; --i) {
    env_sym_t *last = &(env->symbols[i]);
    while (last->next != NULL) {
      if (!strcmp(name, last->name)) {
        return (obj_t*) last->obj;
      }
      last = last->next;
    }

    // Check the last one.
    if (!strcmp(name, last->name)) {
      return (obj_t*) last->obj;
    }
  }

  // Not found :(
  return undef_obj();
}

error_t env_init(env_t *env) {
  for (int i = 0; i < ENV_MAX_SCOPES; ++i) {
    env->symbols[i] = *empty_sym();
  }

  // An outer push_scope() is required.
  env->top = -1;

  return NO_ERROR;
}

