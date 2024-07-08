#pragma once

#include "../common/err.h"
#include "token.h"
#include "lex.h"

enum {
    COMP_ERR_NO_ERROR,
    COMP_UNEXPECTED_TOKEN,
};

/*
 * Following the Principle of Least Astonishment.
 */
typedef enum {
    PRECED_NONE = 0, // Lowest precedence
    PRECED_CAST,
    PRECED_TYPED,
    PRECED_TYPEDEF,
    PRECED_ASSIGN,
    PRECED_COND,
    PRECED_MAPS_TO,
    PRECED_OR,
    PRECED_AND,
    PRECED_EQ,
    PRECED_GLT,
    PRECED_BITWISE_OR,
    PRECED_BITWISE_XOR,
    PRECED_BITWISE_AND,
    PRECED_MEMBERSHIP,
    PRECED_RANGE,
    PRECED_TERM,
    PRECED_FACTOR,
    PRECED_BITWISE_SHIFT,
    PRECED_BITWISE_NOT,
    PRECED_NOT,
    PRECED_SUBSCRIPT,
    PRECED_FUNCTION_CALL,
    PRECED_MEMBER_ACCESS,
} preced_t;

typedef struct parser_t {
    lexer_t *lexer;
    cg_t *cg;
    token_t prev;
    token_t curr;
    uint8_t err;
} parser_t;

error_t comp(const char *input);
