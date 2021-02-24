#include <stdio.h>
#include <string.h>
#include "unity/unity.h"
#include "test_env.h"
#include "../inc/env.h"

void test_env_init() {
  env_t env;
  env_init(&env);

  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "pie")->type);
}

void test_env_no_scope_error() {
  env_t env;
  env_init(&env);

  TEST_ASSERT_EQUAL(ENV_NO_SCOPE, put_env(&env, "foo", int_obj(42)));

  // First, create the initial scope.
  TEST_ASSERT_EQUAL(NO_ERROR, push_scope(&env));
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "foo", int_obj(42)));
}

void test_env_put_get() {
  env_t env;
  env_init(&env);
  push_scope(&env);

  obj_t *not_found = get_env(&env, "ethel");
  TEST_ASSERT_EQUAL(TYPE_UNDEF, not_found->type);

  obj_t *obj = int_obj(42);

  int error = put_env(&env, "ethel", obj);
  TEST_ASSERT_EQUAL(NO_ERROR, error);

  obj_t *found = get_env(&env, "ethel");
  TEST_ASSERT_EQUAL(TYPE_INT, found->type);
  TEST_ASSERT_EQUAL(42, found->intval);
}

void test_env_scopes() {
  env_t env;
  env_init(&env);
  push_scope(&env);
  
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "one-1", float_obj(1.1)));
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "one-2", float_obj(1.2)));
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "one-3", float_obj(1.3)));

  TEST_ASSERT_EQUAL(NO_ERROR, push_scope(&env));
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "two-1", float_obj(2.1)));
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "two-2", float_obj(2.2)));

  TEST_ASSERT_EQUAL(NO_ERROR, push_scope(&env));
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "three-1", float_obj(3.1)));
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "three-2", float_obj(3.2)));
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "three-3", float_obj(3.3)));

  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-4")->type);
  TEST_ASSERT_EQUAL(3.3, get_env(&env, "three-3")->floatval);
  TEST_ASSERT_EQUAL(3.2, get_env(&env, "three-2")->floatval);
  TEST_ASSERT_EQUAL(3.1, get_env(&env, "three-1")->floatval);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "two-3")->type);
  TEST_ASSERT_EQUAL(2.2, get_env(&env, "two-2")->floatval);
  TEST_ASSERT_EQUAL(2.1, get_env(&env, "two-1")->floatval);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "one-4")->type);
  TEST_ASSERT_EQUAL(1.3, get_env(&env, "one-3")->floatval);
  TEST_ASSERT_EQUAL(1.2, get_env(&env, "one-2")->floatval);
  TEST_ASSERT_EQUAL(1.1, get_env(&env, "one-1")->floatval);

  // Pop the topmost scope.
  TEST_ASSERT_EQUAL(NO_ERROR, pop_scope(&env));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-4")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-3")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-2")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-1")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "two-3")->type);
  TEST_ASSERT_EQUAL(2.2, get_env(&env, "two-2")->floatval);
  TEST_ASSERT_EQUAL(2.1, get_env(&env, "two-1")->floatval);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "one-4")->type);
  TEST_ASSERT_EQUAL(1.3, get_env(&env, "one-3")->floatval);
  TEST_ASSERT_EQUAL(1.2, get_env(&env, "one-2")->floatval);
  TEST_ASSERT_EQUAL(1.1, get_env(&env, "one-1")->floatval);

  // Pop the next scope.
  TEST_ASSERT_EQUAL(NO_ERROR, pop_scope(&env));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-4")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-3")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-2")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-1")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "two-3")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "two-2")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "two-1")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "one-4")->type);
  TEST_ASSERT_EQUAL(1.3, get_env(&env, "one-3")->floatval);
  TEST_ASSERT_EQUAL(1.2, get_env(&env, "one-2")->floatval);
  TEST_ASSERT_EQUAL(1.1, get_env(&env, "one-1")->floatval);

  // Pop the last scope. Nothing is in scope.
  TEST_ASSERT_EQUAL(NO_ERROR, pop_scope(&env));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-4")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-3")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-2")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "three-1")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "two-3")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "two-2")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "two-1")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "one-4")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "one-3")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "one-2")->type);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "one-1")->type);

  // Sanity check. We should be out of all scopes now.
  TEST_ASSERT_EQUAL(ENV_NO_SCOPE, put_env(&env, "one-new-1", int_obj(42)));
  TEST_ASSERT_EQUAL(NO_ERROR, push_scope(&env));

  // And we can push things back in new scopes.
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "one-new-1", int_obj(42)));
  TEST_ASSERT_EQUAL(NO_ERROR, push_scope(&env));
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "two-new-1", int_obj(17)));
  TEST_ASSERT_EQUAL(TYPE_INT, get_env(&env, "one-new-1")->type);
  TEST_ASSERT_EQUAL(TYPE_INT, get_env(&env, "two-new-1")->type);
}

void test_env_redefinition_error(void) {
  env_t env;
  env_init(&env);
  push_scope(&env);

  // Not there.
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, "thing")->type);
  
  // There.
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "thing", int_obj(6)));
  TEST_ASSERT_EQUAL(6, get_env(&env, "thing")->intval);

  // Can't define it again at this scope.
  TEST_ASSERT_EQUAL(ENV_SYMBOL_REDEFINED, put_env(&env, "thing", float_obj(1.2)));

  // But can shadow it in a deeper scope.
  push_scope(&env); 
  TEST_ASSERT_EQUAL(NO_ERROR, put_env(&env, "thing", float_obj(1.2)));
}

void test_env(void) {
  RUN_TEST(test_env_init);
  RUN_TEST(test_env_no_scope_error);
  RUN_TEST(test_env_put_get);
  RUN_TEST(test_env_scopes);
  RUN_TEST(test_env_redefinition_error);
}
