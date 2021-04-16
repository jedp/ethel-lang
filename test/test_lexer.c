#include <stdio.h>
#include "unity/unity.h"
#include "test_lexer.h"
#include "../inc/str.h"
#include "../inc/err.h"
#include "../inc/lexer.h"

void test_lex_error(void) {
  char *expr = "if 42 @ moo";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  advance(&lexer); // eat "if"
  advance(&lexer); // eat "42"

  TEST_ASSERT_EQUAL(TAG_EOF, lexer.token.tag);
  TEST_ASSERT_EQUAL(ERR_LEX_UNEXPECTED_TOKEN, lexer.err);
}

void test_lex_word(void) {
  char *expr = "for i in 1 .. 10 step 2";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  token_t expected[] = {
    { .tag = TAG_FOR,     .string = "for" },
    { .tag = TAG_IDENT,   .string = "i" },
    { .tag = TAG_IN,      .string = "in" },
    { .tag = TAG_INT,     .intval = 1 },
    { .tag = TAG_RANGE },
    { .tag = TAG_INT,     .intval = 10 },
    { .tag = TAG_STEP,    .string = "step" },
    { .tag = TAG_INT,     .intval = 2 },
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i].tag, lexer.token.tag);
    if (lexer.token.tag == TAG_INT) {
      TEST_ASSERT_EQUAL(expected[i].intval, lexer.token.intval);
    } else if (lexer.token.tag != TAG_RANGE) {
      TEST_ASSERT(c_str_eq(expected[i].string, lexer.token.string));
    }
    advance(&lexer);
  }
}

void test_lex_hex(void) {
  char *expr = "0xCafE42";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  TEST_ASSERT_EQUAL(TAG_HEX, lexer.token.tag);
  TEST_ASSERT_EQUAL_STRING("cafe42", lexer.token.string);
}

void test_lex_bin(void) {
  char *expr = "0b1101101";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  TEST_ASSERT_EQUAL(TAG_BIN, lexer.token.tag);
  TEST_ASSERT_EQUAL_STRING("1101101", lexer.token.string);
}

void test_lex_int(void) {
  char *expr = "3120";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  TEST_ASSERT_EQUAL(TAG_INT, lexer.token.tag);
  TEST_ASSERT_EQUAL(3120, lexer.token.intval);
}

void test_lex_float(void) {
  char *expr = "3.1415";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  TEST_ASSERT_EQUAL(TAG_FLOAT, lexer.token.tag);
  TEST_ASSERT_EQUAL((float) 3.1415, lexer.token.floatval);
}

void test_lex_float_no_leading_decimal(void) {
  char *expr = ".125";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  TEST_ASSERT_EQUAL(TAG_FLOAT, lexer.token.tag);
  TEST_ASSERT_EQUAL((float) 0.125, lexer.token.floatval);
}

void test_lex_char(void) {
  char *expr = "'c'";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  TEST_ASSERT_EQUAL(TAG_BYTE, lexer.token.tag);
  TEST_ASSERT_EQUAL('c', lexer.token.ch);
}

void test_lex_string(void) {
  char *expr = "\"i like pie\"";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  TEST_ASSERT_EQUAL(TAG_STRING, lexer.token.tag);
  TEST_ASSERT_EQUAL_STRING("i like pie", lexer.token.string);
}

void test_lex_true(void) {
  char *expr = "true";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  TEST_ASSERT_EQUAL(TAG_TRUE, lexer.token.tag);
}

void test_lex_false(void) {
  char *expr = "false";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  TEST_ASSERT_EQUAL(TAG_FALSE, lexer.token.tag);
}

void test_lex_arithmetic(void) {
  char *expr = "123 + 2345.67 * 3 - .42 / 5";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_INT, TAG_PLUS,
    TAG_FLOAT, TAG_TIMES,
    TAG_INT, TAG_MINUS,
    TAG_FLOAT, TAG_DIVIDE,
    TAG_INT, TAG_EOF
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_arithmetic_no_spaces(void) {
  char *expr = "123+2345.67*3-.42/5";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_INT, TAG_PLUS,
    TAG_FLOAT, TAG_TIMES,
    TAG_INT, TAG_MINUS,
    TAG_FLOAT, TAG_DIVIDE,
    TAG_INT, TAG_EOF
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_inequality(void) {
  char *expr = "if a < 1 and b <= 2 and c > 3 and d >= 4";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_IF, TAG_IDENT, TAG_LT, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_LE, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_GT, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_GE, TAG_INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_inequality_no_spaces(void) {
  char *expr = "if a<1 and b<=2 and c>3 and d>=4";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_IF, TAG_IDENT, TAG_LT, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_LE, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_GT, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_GE, TAG_INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_inequality_nested_expressions(void) {
  char *expr = "if (a < 1) and (b<=(2*4)) and c>3 and d>=4";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_IF, TAG_LPAREN, TAG_IDENT, TAG_LT, TAG_INT, TAG_RPAREN,
    TAG_AND, TAG_LPAREN, TAG_IDENT, TAG_LE,
      TAG_LPAREN, TAG_INT, TAG_TIMES, TAG_INT, TAG_RPAREN, TAG_RPAREN,
    TAG_AND, TAG_IDENT, TAG_GT, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_GE, TAG_INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_range(void) {
  char *expr = "1 .. 10";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = { TAG_INT, TAG_RANGE, TAG_INT };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }

  // Also without whitespace, so we don't misconstrue this as a broken float.
  char *expr2 = "1 .. 10";
  lexer_init(&lexer, expr2, c_str_len(expr));
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_parens(void) {
  char *expr = "(1.24 * ( 3.5-2+ 4 ) + 1) * 2";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_LPAREN, TAG_FLOAT, TAG_TIMES,
    TAG_LPAREN, TAG_FLOAT, TAG_MINUS, TAG_INT, TAG_PLUS, TAG_INT, TAG_RPAREN,
    TAG_PLUS, TAG_INT, TAG_RPAREN,
    TAG_TIMES, TAG_INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_parens_no_spaces(void) {
  char *expr = "(1.24*(3.5-2+4)+1)*2";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_LPAREN, TAG_FLOAT, TAG_TIMES,
    TAG_LPAREN, TAG_FLOAT, TAG_MINUS, TAG_INT, TAG_PLUS, TAG_INT, TAG_RPAREN,
    TAG_PLUS, TAG_INT, TAG_RPAREN,
    TAG_TIMES, TAG_INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_assign(void) {
  char *expr = "val a = 3";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = { TAG_INVARIABLE, TAG_IDENT, TAG_ASSIGN, TAG_INT };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_line_with_comment(void) {
  char *expr = "val a = 3 ; a comment ... ";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = { TAG_INVARIABLE, TAG_IDENT, TAG_ASSIGN, TAG_INT };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_only_whitespace_input(void) {
  char *expr = "  ";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  TEST_ASSERT_EQUAL(TAG_EOF, lexer.token.tag);
}

void test_lex_no_input(void) {
  char *expr = "";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  TEST_ASSERT_EQUAL(TAG_EOF, lexer.token.tag);
}

void test_lex_comment_only(void) {
  char *expr = ";";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
  // TODO not sure about this
  TEST_ASSERT_EQUAL(TAG_EOL, lexer.token.tag);
}

void test_lex_begin_end(void) {
  char *expr = "for i in 1 .. 10 {\n  print(i) \n}";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = { 
    TAG_FOR, TAG_IDENT, TAG_IN, TAG_INT, TAG_RANGE, TAG_INT, TAG_BEGIN, TAG_EOL,
    TAG_PRINT, TAG_LPAREN, TAG_IDENT, TAG_RPAREN, TAG_EOL,
    TAG_END, TAG_EOF
  };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_list_of(void) {
  char *expr = "var x = list of glug";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_VARIABLE, TAG_IDENT, TAG_ASSIGN, TAG_LIST, TAG_OF, TAG_TYPE_NAME
  };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_list_of_with_init(void) {
  char *expr = "val x = list of int { 1, 2, 3}";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_INVARIABLE, TAG_IDENT, TAG_ASSIGN, TAG_LIST, TAG_OF, TAG_TYPE_NAME,
    TAG_BEGIN, TAG_INT, TAG_COMMA, TAG_INT, TAG_COMMA, TAG_INT, TAG_END
  };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_member_of(void) {
  char *expr = "val x = 2 in 0..10";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_INVARIABLE, TAG_IDENT, TAG_ASSIGN,
    TAG_INT, TAG_IN, TAG_INT, TAG_RANGE, TAG_INT
  };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_field_access(void) {
  char *expr = "val l = x.length";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_INVARIABLE, TAG_IDENT, TAG_ASSIGN,
    TAG_IDENT, TAG_MEMBER_ACCESS, TAG_FIELD_NAME
  };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);

    if (lexer.token.tag == TAG_FIELD_NAME) {
      TEST_ASSERT_EQUAL_STRING("length", lexer.token.string);
    }

    advance(&lexer);
  }
}

void test_lex_method_access(void) {
  char *expr = "foo.insert(42)";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_IDENT, TAG_MEMBER_ACCESS, TAG_METHOD_CALL, TAG_LPAREN, TAG_INT, TAG_RPAREN
  };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);

    if (lexer.token.tag == TAG_METHOD_CALL) {
      TEST_ASSERT_EQUAL_STRING("insert", lexer.token.string);
    }
    advance(&lexer);
  }
}

void test_lex_array_access(void) {
  char *expr = "val e = a[42]";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_INVARIABLE, TAG_IDENT, TAG_ASSIGN,
    TAG_IDENT, TAG_LBRACKET, TAG_INT, TAG_RBRACKET
  };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_array_constructor(void) {
  char *expr = "val a = arr(16)";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_INVARIABLE, TAG_IDENT, TAG_ASSIGN,
    TAG_ARR_DECL, TAG_LPAREN, TAG_INT, TAG_RPAREN
  };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_function_definition(void) {
  char *expr = "val f = fn(x, y) { x + y }";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_INVARIABLE, TAG_IDENT, TAG_ASSIGN,
    TAG_FUNC_DEF, TAG_LPAREN, TAG_IDENT, TAG_COMMA, TAG_IDENT, TAG_RPAREN,
    TAG_BEGIN, TAG_IDENT, TAG_PLUS, TAG_IDENT, TAG_END
  };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_function_call(void) {
  char* expr = "val x = f(42)";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_INVARIABLE, TAG_IDENT, TAG_ASSIGN,
    TAG_FUNC_CALL, TAG_LPAREN, TAG_INT, TAG_RPAREN
  };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_function_with_return(void) {
  char* expr = "val x = fn(x) { return x + 1 }";
  lexer_t lexer;
  lexer_init(&lexer, expr, c_str_len(expr));

  int expected[] = {
    TAG_INVARIABLE, TAG_IDENT, TAG_ASSIGN,
    TAG_FUNC_DEF, TAG_LPAREN, TAG_IDENT, TAG_RPAREN,
    TAG_BEGIN, TAG_FUNC_RETURN, TAG_IDENT, TAG_PLUS, TAG_INT, TAG_END
  };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_all_tokens(void) {
  typedef struct {
    char* text;
    int expected_tag;
  } test_data_t;

  test_data_t test_data[] = {
    (test_data_t) { .text = "fn", .expected_tag = TAG_FUNC_DEF },
    (test_data_t) { .text = "return", .expected_tag = TAG_FUNC_RETURN },
    (test_data_t) { .text = "x", .expected_tag = TAG_IDENT },
    (test_data_t) { .text = "=", .expected_tag = TAG_ASSIGN },
    (test_data_t) { .text = "{", .expected_tag = TAG_BEGIN },
    (test_data_t) { .text = "}", .expected_tag = TAG_END },
    (test_data_t) { .text = "(", .expected_tag = TAG_LPAREN },
    (test_data_t) { .text = ")", .expected_tag = TAG_RPAREN },
    (test_data_t) { .text = ",", .expected_tag = TAG_COMMA },
    (test_data_t) { .text = "+", .expected_tag = TAG_PLUS },
    (test_data_t) { .text = "-", .expected_tag = TAG_MINUS },
    (test_data_t) { .text = "*", .expected_tag = TAG_TIMES },
    (test_data_t) { .text = "/", .expected_tag = TAG_DIVIDE },
    (test_data_t) { .text = "&", .expected_tag = TAG_BITWISE_AND },
    (test_data_t) { .text = "|", .expected_tag = TAG_BITWISE_OR },
    (test_data_t) { .text = "^", .expected_tag = TAG_BITWISE_XOR },
    (test_data_t) { .text = "~", .expected_tag = TAG_BITWISE_NOT },
    (test_data_t) { .text = "<<", .expected_tag = TAG_BITWISE_SHL },
    (test_data_t) { .text = ">>", .expected_tag = TAG_BITWISE_SHR },
    (test_data_t) { .text = "==", .expected_tag = TAG_EQ },
    (test_data_t) { .text = "!=", .expected_tag = TAG_NE },
    (test_data_t) { .text = "<", .expected_tag = TAG_LT },
    (test_data_t) { .text = "<=", .expected_tag = TAG_LE },
    (test_data_t) { .text = ">", .expected_tag = TAG_GT },
    (test_data_t) { .text = ">=", .expected_tag = TAG_GE },
    (test_data_t) { .text = "and", .expected_tag = TAG_AND },
    (test_data_t) { .text = "or", .expected_tag = TAG_OR },
    (test_data_t) { .text = "mod", .expected_tag = TAG_MOD },
    (test_data_t) { .text = "42", .expected_tag = TAG_INT },
    (test_data_t) { .text = "3.14", .expected_tag = TAG_FLOAT },
    (test_data_t) { .text = "'c'", .expected_tag = TAG_BYTE },
    (test_data_t) { .text = "\"string\"", .expected_tag = TAG_STRING },
    (test_data_t) { .text = "list", .expected_tag = TAG_LIST },
    (test_data_t) { .text = "of", .expected_tag = TAG_OF },
    (test_data_t) { .text = "if", .expected_tag = TAG_IF },
    (test_data_t) { .text = "then", .expected_tag = TAG_THEN },
    (test_data_t) { .text = "else", .expected_tag = TAG_ELSE },
    (test_data_t) { .text = "while", .expected_tag = TAG_WHILE },
    (test_data_t) { .text = "for", .expected_tag = TAG_FOR },
    (test_data_t) { .text = "in", .expected_tag = TAG_IN },
    (test_data_t) { .text = "match", .expected_tag = TAG_MATCH },
    (test_data_t) { .text = "step", .expected_tag = TAG_STEP },
    (test_data_t) { .text = "hex", .expected_tag = TAG_TO_HEX },
    (test_data_t) { .text = "bin", .expected_tag = TAG_TO_BIN },

  };

  int num_data = sizeof(test_data) / sizeof(test_data[0]);
  lexer_t lexer;
  for (int i = 0; i < num_data; ++i) {
    lexer_init(&lexer, test_data[i].text, c_str_len(test_data[i].text));
    TEST_ASSERT_EQUAL(test_data[i].expected_tag, lexer.token.tag);
  }
}

void test_lexer(void) {
  RUN_TEST(test_lex_error);
  RUN_TEST(test_lex_word);
  RUN_TEST(test_lex_int);
  RUN_TEST(test_lex_hex);
  RUN_TEST(test_lex_bin);
  RUN_TEST(test_lex_float);
  RUN_TEST(test_lex_float_no_leading_decimal);
  RUN_TEST(test_lex_char);
  RUN_TEST(test_lex_string);
  RUN_TEST(test_lex_true);
  RUN_TEST(test_lex_false);
  RUN_TEST(test_lex_arithmetic);
  RUN_TEST(test_lex_arithmetic_no_spaces);
  RUN_TEST(test_lex_inequality);
  RUN_TEST(test_lex_inequality_nested_expressions);
  RUN_TEST(test_lex_range);
  RUN_TEST(test_lex_parens);
  RUN_TEST(test_lex_parens_no_spaces);
  RUN_TEST(test_lex_assign);
  RUN_TEST(test_lex_line_with_comment);
  RUN_TEST(test_lex_no_input);
  RUN_TEST(test_lex_only_whitespace_input);
  RUN_TEST(test_lex_comment_only);
  RUN_TEST(test_lex_begin_end);
  RUN_TEST(test_lex_list_of);
  RUN_TEST(test_lex_list_of_with_init);
  RUN_TEST(test_lex_member_of);
  RUN_TEST(test_lex_field_access);
  RUN_TEST(test_lex_method_access);
  RUN_TEST(test_lex_array_access);
  RUN_TEST(test_lex_array_constructor);
  RUN_TEST(test_lex_function_definition);
  RUN_TEST(test_lex_function_call);
  RUN_TEST(test_lex_function_with_return);
  RUN_TEST(test_lex_all_tokens);
}

