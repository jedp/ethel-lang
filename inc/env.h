#ifndef __ENV_H
#define __ENV_H

#include "ast.h"

#define ENV_MAX_SCOPES 16

typedef enum {
  ENV_NO_ERROR = 0,
  ENV_MAX_DEPTH_EXCEEDED,
  ENV_NO_SCOPE,
  ENV_SYMBOL_REDEFINITION,
  ENV_SYMBOL_UNDEFINED,
} env_error_t;

typedef struct Symbol {
    const char* name;
    const struct Symbol *expr;
    struct Symbol *next;
} env_sym_t;

/* A stack of symbol tables in scope. */
typedef struct Env {
    int top;
    env_sym_t symbols[ENV_MAX_SCOPES];
} env_t;

env_error_t env_init(env_t *env);
env_error_t push_scope(env_t *env);
env_error_t pop_scope(env_t *env);
env_error_t put_env(env_t *env, const char* name, const ast_expr_t *expr);
ast_expr_t *get_env(env_t *env, const char* name);

#endif

