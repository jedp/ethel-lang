#include <stdio.h>
#include "unity/unity.h"
#include "test_env.h"
#include "../inc/type.h"
#include "../inc/str.h"
#include "../inc/env.h"

#define NAME(x) (string_obj(c_str_to_bytearray(x)))

void test_env_init() {
  env_t env;
  env_init(&env);

  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("pie"))));
}

void test_env_no_scope_error() {
  env_t env;
  env_init(&env);

  TEST_ASSERT_EQUAL(ERR_ENV_NO_SCOPE, put_env(&env, NAME("foo"), int_obj(42), F_NONE));

  // First, create the initial scope.
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, enter_scope(&env));
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("foo"), int_obj(42), F_NONE));
}

void test_env_put_get() {
  env_t env;
  env_init(&env);
  enter_scope(&env);

  obj_t *not_found = get_env(&env, NAME("ethel"));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(not_found));

  obj_t *obj = int_obj(42);

  int error = put_env(&env, NAME("ethel"), obj, F_NONE);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, error);

  obj_t *found = get_env(&env, NAME("ethel"));
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(found));
  TEST_ASSERT_EQUAL(42, found->intval);
}

void test_env_put_del_get() {
  env_t env;
  env_init(&env);
  enter_scope(&env);

  obj_t *not_found = get_env(&env, NAME("ethel"));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(not_found));

  obj_t *obj = int_obj(42);

  int error = put_env(&env, NAME("ethel"), obj, F_NONE);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, error);

  obj_t *found = get_env(&env, NAME("ethel"));
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(found));
  TEST_ASSERT_EQUAL(42, found->intval);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, del_env(&env, NAME("ethel")));

  not_found = get_env(&env, NAME("ethel"));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(not_found));
}

void test_env_scopes() {
  env_t env;
  env_init(&env);
  enter_scope(&env);

  TEST_ASSERT_EQUAL(0, env.top);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("one-1"), float_obj(1.1), F_NONE));
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("one-2"), float_obj(1.2), F_NONE));
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("one-3"), float_obj(1.3), F_NONE));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, enter_scope(&env));
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("two-1"), float_obj(2.1), F_NONE));
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("two-2"), float_obj(2.2), F_NONE));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, enter_scope(&env));
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("three-1"), float_obj(3.1), F_NONE));
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("three-2"), float_obj(3.2), F_NONE));
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("three-3"), float_obj(3.3), F_NONE));

  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-4"))));
  TEST_ASSERT_EQUAL(3.3, get_env(&env, NAME("three-3"))->floatval);
  TEST_ASSERT_EQUAL(3.2, get_env(&env, NAME("three-2"))->floatval);
  TEST_ASSERT_EQUAL(3.1, get_env(&env, NAME("three-1"))->floatval);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("two-3"))));
  TEST_ASSERT_EQUAL(2.2, get_env(&env, NAME("two-2"))->floatval);
  TEST_ASSERT_EQUAL(2.1, get_env(&env, NAME("two-1"))->floatval);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("one-4"))));
  TEST_ASSERT_EQUAL(1.3, get_env(&env, NAME("one-3"))->floatval);
  TEST_ASSERT_EQUAL(1.2, get_env(&env, NAME("one-2"))->floatval);
  TEST_ASSERT_EQUAL(1.1, get_env(&env, NAME("one-1"))->floatval);

  // Pop the topmost scope.
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, leave_scope(&env));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-4"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-3"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-2"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-1"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("two-3"))));
  TEST_ASSERT_EQUAL(2.2, get_env(&env, NAME("two-2"))->floatval);
  TEST_ASSERT_EQUAL(2.1, get_env(&env, NAME("two-1"))->floatval);
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("one-4"))));
  TEST_ASSERT_EQUAL(1.3, get_env(&env, NAME("one-3"))->floatval);
  TEST_ASSERT_EQUAL(1.2, get_env(&env, NAME("one-2"))->floatval);
  TEST_ASSERT_EQUAL(1.1, get_env(&env, NAME("one-1"))->floatval);

  // Pop the next scope.
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, leave_scope(&env));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-4"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-3"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-2"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-1"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("two-3"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("two-2"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("two-1"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("one-4"))));
  TEST_ASSERT_EQUAL(1.3, get_env(&env, NAME("one-3"))->floatval);
  TEST_ASSERT_EQUAL(1.2, get_env(&env, NAME("one-2"))->floatval);
  TEST_ASSERT_EQUAL(1.1, get_env(&env, NAME("one-1"))->floatval);

  // Pop the last scope. Nothing is in scope.
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, leave_scope(&env));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-4"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-3"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-2"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("three-1"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("two-3"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("two-2"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("two-1"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("one-4"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("one-3"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("one-2"))));
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("one-1"))));

  // Sanity check. We should be out of all scopes now.
  TEST_ASSERT_EQUAL(ERR_ENV_NO_SCOPE, put_env(&env, NAME("one-new-1"), int_obj(42), F_NONE));
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, enter_scope(&env));
  TEST_ASSERT_EQUAL(0, env.top);

  // And we can push things back in new scopes.
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("one-new-1"), int_obj(42), F_NONE));
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, enter_scope(&env));
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("two-new-1"), int_obj(17), F_NONE));
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(get_env(&env, NAME("one-new-1"))));
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(get_env(&env, NAME("two-new-1"))));
}

void test_env_redefinition_error(void) {
  env_t env;
  env_init(&env);
  enter_scope(&env);

  // Not there.
  TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("thing"))));

  // There.
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&env, NAME("thing"), int_obj(6), F_NONE));
  TEST_ASSERT_EQUAL(6, get_env(&env, NAME("thing"))->intval);

  // Can't define it again at this scope.
  TEST_ASSERT_EQUAL(ERR_ENV_SYMBOL_REDEFINED, put_env(&env, NAME("thing"), float_obj(1.2), F_NONE));

  // Also cannot shadow it in a deeper scope.
  enter_scope(&env);
  TEST_ASSERT_EQUAL(ERR_ENV_SYMBOL_REDEFINED, put_env(&env, NAME("thing"), float_obj(1.2), F_NONE));
}

void test_env(void) {
  RUN_TEST(test_env_init);
  RUN_TEST(test_env_no_scope_error);
  RUN_TEST(test_env_put_get);
  RUN_TEST(test_env_put_del_get);
  RUN_TEST(test_env_scopes);
  RUN_TEST(test_env_redefinition_error);
}
