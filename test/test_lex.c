#include <string.h>

#include "unity/unity.h"
#include "test_lex.h"
#include "../src/comp/lex.h"

static void expect_type(lexer_t *lexer, tag_t tag) {
    TEST_ASSERT_EQUAL(tag, next_token(lexer).tag);
}

static void expect_error(lexer_t *lexer, uint8_t err) {
    token_t token = next_token(lexer);
    TEST_ASSERT_EQUAL(TAG_ERROR, token.tag);
    TEST_ASSERT_EQUAL(err, token.err);
}

static void expect_type_with_text(lexer_t *lexer, tag_t tag, const char *expected) {
    TEST_ASSERT_EQUAL(tag, next_token(lexer).tag);
    char actual[lexer->curr - lexer->start + 1];
    memcpy(actual, lexer->start, strlen(expected) + 1);
    actual[strlen(expected)] = '\0';
    //printf("expected '%s'; got '%s'\n", expected, actual);
    TEST_ASSERT_EQUAL(0, memcmp(lexer->start, expected, strlen(expected)));
}

void test_lex_eof(void) {
    char *expr = "hi";
    lexer_t lexer;
    lexer_init(&lexer, expr);

    expect_type_with_text(&lexer, TAG_IDENT, "hi");
    expect_type(&lexer, TAG_EOF);
    // Ok to keep requesting next token. Still at EOF.
    expect_type(&lexer, TAG_EOF);
    expect_type(&lexer, TAG_EOF);
}

void test_lex_tokenize(void) {
    char *expr = "identifier Boolean Byte Bytearray Float Int List Map String   \n"
                 "abs and as bin break continue cos data del do dump else env   \n"
                 "exp false for fun hex if in input is ln log match mem nil not \n"
                 "or print rand read return sin sqrt step struct tan then true  \n"
                 "type typeof val var while { } [ ] , : + * - / % & | ^ ~ ..    \n"
                 ".glug .merg() < <= << > >= >> = == ! != 'c' \"str\" 42 3.14   \n"
                 "0xff 0b01100101 // Ignore comment here, but keep the newline  \n"
                 "// Ignore comment at end.";
    lexer_t lexer;
    lexer_init(&lexer, expr);

    /* "identifier Boolean Byte Bytearray Float Int List Map String \n" */
    expect_type_with_text(&lexer, TAG_IDENT, "identifier");
    expect_type(&lexer, TAG_TYPE_BOOLEAN);
    expect_type(&lexer, TAG_TYPE_BYTE);
    expect_type(&lexer, TAG_TYPE_BYTEARRAY);
    expect_type(&lexer, TAG_TYPE_FLOAT);
    expect_type(&lexer, TAG_TYPE_INT);
    expect_type(&lexer, TAG_TYPE_LIST);
    expect_type(&lexer, TAG_TYPE_MAP);
    expect_type(&lexer, TAG_TYPE_STRING);
    expect_type(&lexer, TAG_EOL);

    /* "abs and as bin break continue cos data del do dump else env \n" */
    expect_type(&lexer, TAG_ABS);
    expect_type(&lexer, TAG_AND);
    expect_type(&lexer, TAG_AS);
    expect_type(&lexer, TAG_BIN);
    expect_type(&lexer, TAG_BREAK);
    expect_type(&lexer, TAG_CONTINUE);
    expect_type(&lexer, TAG_COS);
    expect_type(&lexer, TAG_DATA);
    expect_type(&lexer, TAG_DEL);
    expect_type(&lexer, TAG_DO);
    expect_type(&lexer, TAG_DUMP);
    expect_type(&lexer, TAG_ELSE);
    expect_type(&lexer, TAG_ENV);
    expect_type(&lexer, TAG_EOL);

    /* "exp false for fun hex if in input is ln log match mem nil not \n" */
    expect_type(&lexer, TAG_EXP);
    expect_type(&lexer, TAG_FALSE);
    expect_type(&lexer, TAG_FOR);
    expect_type(&lexer, TAG_FUNC_DEF);
    expect_type(&lexer, TAG_HEX);
    expect_type(&lexer, TAG_IF);
    expect_type(&lexer, TAG_IN);
    expect_type(&lexer, TAG_INPUT);
    expect_type(&lexer, TAG_IS);
    expect_type(&lexer, TAG_LN);
    expect_type(&lexer, TAG_LOG);
    expect_type(&lexer, TAG_MATCH);
    expect_type(&lexer, TAG_MEM);
    expect_type(&lexer, TAG_NIL);
    expect_type(&lexer, TAG_NOT);
    expect_type(&lexer, TAG_EOL);

    /* "or print rand read return sin sqrt step struct tan then true \n" */
    expect_type(&lexer, TAG_OR);
    expect_type(&lexer, TAG_PRINT);
    expect_type(&lexer, TAG_RAND);
    expect_type(&lexer, TAG_READ);
    expect_type(&lexer, TAG_FUNC_RETURN);
    expect_type(&lexer, TAG_SIN);
    expect_type(&lexer, TAG_SQRT);
    expect_type(&lexer, TAG_STEP);
    expect_type(&lexer, TAG_TYPEDEF_STRUCT);
    expect_type(&lexer, TAG_TAN);
    expect_type(&lexer, TAG_THEN);
    expect_type(&lexer, TAG_TRUE);
    expect_type(&lexer, TAG_EOL);

    /* "type typeof val var while { } [ ] , : + * - / % & | ^ ~ .. \n" */
    expect_type(&lexer, TAG_TYPE);
    expect_type(&lexer, TAG_TYPEOF);
    expect_type(&lexer, TAG_INVARIABLE);
    expect_type(&lexer, TAG_VARIABLE);
    expect_type(&lexer, TAG_WHILE);
    expect_type(&lexer, TAG_BEGIN);
    expect_type(&lexer, TAG_END);
    expect_type(&lexer, TAG_LBRACKET);
    expect_type(&lexer, TAG_RBRACKET);
    expect_type(&lexer, TAG_COMMA);
    expect_type(&lexer, TAG_COLON);
    expect_type(&lexer, TAG_PLUS);
    expect_type(&lexer, TAG_TIMES);
    expect_type(&lexer, TAG_MINUS);
    expect_type(&lexer, TAG_DIVIDE);
    expect_type(&lexer, TAG_MOD);
    expect_type(&lexer, TAG_BITWISE_AND);
    expect_type(&lexer, TAG_BITWISE_OR);
    expect_type(&lexer, TAG_BITWISE_XOR);
    expect_type(&lexer, TAG_BITWISE_NOT);
    expect_type(&lexer, TAG_RANGE);
    expect_type(&lexer, TAG_EOL);

    /* ".glug .merg() < <= << > >= >> = == ! != 'c' \"str\" 42 3.14 \n" */
    expect_type_with_text(&lexer, TAG_FIELD_ACCESS, ".glug");
    expect_type_with_text(&lexer, TAG_METHOD_CALL, ".merg");
    expect_type(&lexer, TAG_LPAREN);
    expect_type(&lexer, TAG_RPAREN);
    expect_type(&lexer, TAG_LT);
    expect_type(&lexer, TAG_LE);
    expect_type(&lexer, TAG_BITWISE_SHL);
    expect_type(&lexer, TAG_GT);
    expect_type(&lexer, TAG_GE);
    expect_type(&lexer, TAG_BITWISE_SHR);
    expect_type(&lexer, TAG_ASSIGN);
    expect_type(&lexer, TAG_EQ);
    expect_type(&lexer, TAG_NOT);
    expect_type(&lexer, TAG_NE);
    expect_type_with_text(&lexer, TAG_CHAR, "'c'");
    expect_type_with_text(&lexer, TAG_STRING, "\"str\"");
    expect_type_with_text(&lexer, TAG_INT, "42");
    expect_type_with_text(&lexer, TAG_FLOAT, "3.14");
    expect_type(&lexer, TAG_EOL);

    /* "0xff 0b01100101 // Ignore comment here, but keep the newline  \n" */
    expect_type_with_text(&lexer, TAG_HEX, "0xff");
    expect_type_with_text(&lexer, TAG_BIN, "0b01100101");
    expect_type(&lexer, TAG_EOL);

    /* "// Ignore comment at end."; */
    expect_type(&lexer, TAG_EOF);
}

void test_lex_error(void) {
    char *expr = "if 42 @ moo";
    lexer_t lexer;
    lexer_init(&lexer, expr);

    expect_type(&lexer, TAG_IF);
    expect_type_with_text(&lexer, TAG_INT, "42");
    expect_error(&lexer, LEX_ERR_INVALID_SEQUENCE);
}

void test_lex_indent(void) {
    char *expr = "a                 \n"
                 "    b             \n"
                 "        // Ignore \n"
                 "        c         \n"
                 "        d         \n"
                 "    e             \n"
                 "  f               \n";
    lexer_t lexer;
    lexer_init(&lexer, expr);

    expect_type_with_text(&lexer, TAG_IDENT, "a");
    expect_type(&lexer, TAG_EOL);

    expect_type_with_text(&lexer, TAG_INDENT, "    ");
    expect_type_with_text(&lexer, TAG_IDENT, "b");
    expect_type(&lexer, TAG_EOL);

    expect_type_with_text(&lexer, TAG_INDENT, "        ");
    // Comment ignored, but it did contain a newline.
    expect_type(&lexer, TAG_EOL);

    expect_type_with_text(&lexer, TAG_INDENT, "        ");
    expect_type_with_text(&lexer, TAG_IDENT, "c");
    expect_type(&lexer, TAG_EOL);

    expect_type_with_text(&lexer, TAG_INDENT, "        ");
    expect_type_with_text(&lexer, TAG_IDENT, "d");
    expect_type(&lexer, TAG_EOL);

    expect_type_with_text(&lexer, TAG_INDENT, "    ");
    expect_type_with_text(&lexer, TAG_IDENT, "e");
    expect_type(&lexer, TAG_EOL);

    expect_type_with_text(&lexer, TAG_INDENT, "  ");
    expect_type_with_text(&lexer, TAG_IDENT, "f");
    expect_type(&lexer, TAG_EOL);
}

void test_lex(void) {
    RUN_TEST(test_lex_eof);
    RUN_TEST(test_lex_tokenize);
    RUN_TEST(test_lex_error);
    RUN_TEST(test_lex_indent);
}
