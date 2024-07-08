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

/*
int op_preced_inc(token_t *token) {
    (void) *token;
    // Return -1 for right-associative ops
    return 1;
}

uint8_t token_op_preced(token_t *token) {
    switch (token->tag) {
        case TAG_MEMBER_ACCESS:
            return PRECED_MEMBER_ACCESS;
        case TAG_LPAREN:
            return PRECED_FUNCTION_CALL;
        case TAG_LBRACKET:
            return PRECED_SUBSCRIPT;
        case TAG_AS:
            return PRECED_CAST;
        case TAG_TIMES:
        case TAG_DIVIDE:
        case TAG_MOD:
            return PRECED_FACTOR;
        case TAG_PLUS:
        case TAG_MINUS:
            return PRECED_TERM;
        case TAG_NOT:
            return PRECED_NOT;
        case TAG_GT:
        case TAG_GE:
        case TAG_LT:
        case TAG_LE:
            return PRECED_GLT;
        case TAG_EQ:
        case TAG_NE:
        case TAG_IS:
            return PRECED_EQ;
        case TAG_IN:
            return PRECED_MEMBERSHIP;
        case TAG_BITWISE_AND:
            return PRECED_BITWISE_AND;
        case TAG_BITWISE_XOR:
            return PRECED_BITWISE_XOR;
        case TAG_BITWISE_OR:
            return PRECED_BITWISE_OR;
        case TAG_BITWISE_NOT:
            return PRECED_BITWISE_NOT;
        case TAG_BITWISE_SHL:
        case TAG_BITWISE_SHR:
            return PRECED_BITWISE_SHIFT;
        case TAG_AND:
            return PRECED_AND;
        case TAG_OR:
            return PRECED_OR;
        case TAG_RANGE:
        case TAG_STEP:
            return PRECED_RANGE;
        case TAG_MAPS_TO:
            return PRECED_MAPS_TO;
        case TAG_ASSIGN:
            return PRECED_ASSIGN;
        case TAG_COLON:
            return PRECED_TYPED;
        case TAG_TYPEDEF:
            return PRECED_TYPEDEF;
        default:
            return PRECED_NONE;
    }
}
 */

typedef struct parser_t {
    lexer_t *lexer;
    cg_t *cg;
    token_t prev;
    token_t curr;
    uint8_t err;
} parser_t;

error_t comp(const char *input);
