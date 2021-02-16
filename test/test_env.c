#include <stdio.h>
#include <string.h>
#include "unity/unity.h"
#include "test_env.h"
#include "../inc/env.h"

void test_env_init() {
  env_t env;
  env_init(&env);

  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "pie")->type);
}

void test_env_no_scope_error() {
  env_t env;
  env_init(&env);

  TEST_ASSERT_EQUAL(ENV_NO_SCOPE, put_env(&env, "foo", ast_ident("foo")));

  // First, create the initial scope.
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, push_scope(&env));
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, put_env(&env, "foo", ast_ident("foo")));
}

void test_env_put_get() {
  env_t env;
  env_init(&env);
  push_scope(&env);

  ast_expr_t *not_found = get_env(&env, "ethel");
  TEST_ASSERT_EQUAL(AST_EMPTY, not_found->type);

  ast_expr_t *expr = ast_ident("ethel");
  TEST_ASSERT_EQUAL("ethel", expr->stringval);
  TEST_ASSERT_EQUAL(AST_IDENT, expr->type);

  int error = put_env(&env, "ethel", expr);
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, error);

  ast_expr_t *found = get_env(&env, "ethel");
  TEST_ASSERT_EQUAL(AST_IDENT, found->type);
}

void test_env_scopes() {
  env_t env;
  env_init(&env);
  push_scope(&env);
  
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, put_env(&env, "one-1", ast_ident("one-1")));
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, put_env(&env, "one-2", ast_ident("one-2")));
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, put_env(&env, "one-3", ast_ident("one-3")));

  TEST_ASSERT_EQUAL(ENV_NO_ERROR, push_scope(&env));
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, put_env(&env, "two-1", ast_ident("two-1")));
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, put_env(&env, "two-2", ast_ident("two-2")));

  TEST_ASSERT_EQUAL(ENV_NO_ERROR, push_scope(&env));
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, put_env(&env, "three-1", ast_ident("three-1")));
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, put_env(&env, "three-2", ast_ident("three-2")));
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, put_env(&env, "three-3", ast_ident("three-3")));

  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-4")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "three-3")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "three-2")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "three-1")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "two-3")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "two-2")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "two-1")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "one-4")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "one-3")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "one-2")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "one-1")->type);

  // Pop the topmost scope.
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, pop_scope(&env));
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-4")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-3")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-2")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-1")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "two-3")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "two-2")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "two-1")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "one-4")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "one-3")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "one-2")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "one-1")->type);

  // Pop the next scope.
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, pop_scope(&env));
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-4")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-3")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-2")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-1")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "two-3")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "two-2")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "two-1")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "one-4")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "one-3")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "one-2")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "one-1")->type);

  // Pop the last scope. Nothing is in scope.
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, pop_scope(&env));
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-4")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-3")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-2")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "three-1")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "two-3")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "two-2")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "two-1")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "one-4")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "one-3")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "one-2")->type);
  TEST_ASSERT_EQUAL(AST_EMPTY, get_env(&env, "one-1")->type);

  // Sanity check. We should be out of all scopes now.
  TEST_ASSERT_EQUAL(ENV_NO_SCOPE, put_env(&env, "one-new-1", ast_ident("one-new-1")));
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, push_scope(&env));

  // And we can push things back in new scopes.
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, put_env(&env, "one-new-1", ast_ident("one-new-1")));
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, push_scope(&env));
  TEST_ASSERT_EQUAL(ENV_NO_ERROR, put_env(&env, "two-new-1", ast_ident("two-new-1")));
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "one-new-1")->type);
  TEST_ASSERT_EQUAL(AST_IDENT, get_env(&env, "two-new-1")->type);
}

void test_env(void) {
  RUN_TEST(test_env_init);
  RUN_TEST(test_env_no_scope_error);
  RUN_TEST(test_env_put_get);
  RUN_TEST(test_env_scopes);
}
