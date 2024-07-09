#pragma once

#include "comp.h"
#include "token.h"

/*
 * Following the Principle of Least Astonishment.
 */
typedef enum {
    PRECED_EOF = 0,
    PRECED_NONE, // Lowest precedence
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
    PRECED_UNARY,
    PRECED_BITWISE_SHIFT,
    PRECED_BITWISE_NOT,
    PRECED_NOT,
    PRECED_SUBSCRIPT,
    PRECED_ARGLIST,
    PRECED_MEMBER_ACCESS,
} preced_t;

typedef void (*parse_func)(parser_t *parser);

typedef struct {
    parse_func parse_prefix;
    parse_func parse_infix;
    preced_t precedence;
} parse_preced_rule_t;

void parse_int(parser_t *parser);

void parse_unary_op(parser_t *parser);

void parse_binary_op(parser_t *parser);

void parse_parens(parser_t *parser);

void parse_subscript(parser_t *parser);

void parse_expr_by_precedence(parser_t *parser, uint8_t min_preded);

parse_preced_rule_t preced_rules[] = {
    [TAG_LPAREN] = {parse_parens, NULL, PRECED_ARGLIST},
    [TAG_RPAREN] = {NULL, NULL, PRECED_NONE},
    [TAG_LBRACKET] = {parse_subscript, NULL, PRECED_NONE},
    [TAG_RBRACKET] = {NULL, NULL, PRECED_NONE},
    [TAG_MINUS] = {parse_unary_op, parse_binary_op, PRECED_FACTOR},
    [TAG_PLUS] = {NULL, parse_binary_op, PRECED_FACTOR},
    [TAG_TIMES] = {NULL, parse_binary_op, PRECED_TERM},
    [TAG_DIVIDE]= {NULL, parse_binary_op, PRECED_TERM},
    [TAG_INT] = {parse_int, NULL, PRECED_NONE},
    [TAG_EOF]= {NULL, NULL, PRECED_EOF},
};