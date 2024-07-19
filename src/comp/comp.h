#pragma once

#include "../common/err.h"
#include "cg.h"
#include "token.h"
#include "lex.h"

enum {
    COMP_ERR_NO_ERROR,
    COMP_UNEXPECTED_TOKEN,
    COMP_UNHANDLED_PREFIX_OP,
    COMP_UNHANDLED_INFIX_OP,
    COMP_EXPECTED_EXPRESSION,
};

typedef struct parser_t {
    lexer_t *lexer;
    cg_t *cg;
    token_t prev;
    token_t curr;
    uint8_t err;
} parser_t;

error_t comp(const char *input, cg_t *cg);
