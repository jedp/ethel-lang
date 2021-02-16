#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "../inc/env.h"

env_sym_t *new_sym(const char* name) {
  env_sym_t *sym = malloc(sizeof(env_sym_t));
  sym->name = name;
  sym->expr = (struct Symbol*) ast_empty();
  sym->next = NULL; 

  return sym;
}

env_sym_t *empty_sym() {
  return new_sym("");
}

env_error_t push_scope(env_t *env) {
  env->top += 1;
  if (env->top == ENV_MAX_SCOPES) {
    return ENV_MAX_DEPTH_EXCEEDED;
  }

  env->symbols[env->top] = *empty_sym();

  return ENV_NO_ERROR;
}

env_error_t pop_scope(env_t *env) {
  env->symbols[env->top] = *empty_sym();
  env->top -= 1;

  return ENV_NO_ERROR;
}

env_error_t put_env(env_t *env, const char* name, const ast_expr_t *expr) {
  if (env->top < 0) {
    return ENV_NO_SCOPE;
  }

  env_sym_t *last = &(env->symbols[env->top]);
  while (last->next != NULL) {
    if (!strcmp(name, last->name)) {
      return ENV_SYMBOL_REDEFINITION;
    }
    last = last->next;
  }

  env_sym_t *sym = new_sym(name);
  sym->expr = (struct Symbol*) expr;
  last->next = sym;

  return ENV_NO_ERROR;
}

ast_expr_t *get_env(env_t *env, const char* name) {
  // Search back through the scopes to find the name.
  for (int i = env->top; i >= 0; --i) {
    env_sym_t *last = &(env->symbols[i]);
    while (last != NULL) {
      if (!strcmp(name, last->name)) {
        return (ast_expr_t*) last->expr;
      }
      last = last->next;
    }
  }

  // Not found :(
  return ast_empty();
}

env_error_t env_init(env_t *env) {     
  for (int i = 0; i < ENV_MAX_SCOPES; ++i) {
    env->symbols[i] = *empty_sym();
  }

  // An outer push_scope() is required.
  env->top = -1;

  return ENV_NO_ERROR;
}

