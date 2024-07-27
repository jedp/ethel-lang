#pragma once

#include "comp.h"
#include "token.h"

typedef enum {
    ASSOC_RIGHT = -1,
    ASSOC_LEFT = 1,
} assoc_t;

/*
 * Following the Principle of Least Astonishment.
 */
typedef enum {
    PRECED_EOF = 0,
    PRECED_NONE, // Lowest precedence
    PRECED_CAST,
    PRECED_TYPE_HINT,
    PRECED_TYPEDEF,
    PRECED_ASSIGN,
    PRECED_COND,
    PRECED_MAPS_TO,
    PRECED_LOGICAL_OR,
    PRECED_LOGICAL_AND,
    PRECED_CMP,
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
    PRECED_SUBSCRIPT,
    PRECED_GROUPING,
    PRECED_FUNCTION_ARGS,
    PRECED_MEMBER_ACCESS,
} preced_t;

typedef void (*parse_func)(parser_t *parser);

typedef struct {
    parse_func parse_prefix;
    parse_func parse_infix;
    preced_t precedence;
    assoc_t associativity;

} parse_preced_rule_t;

__attribute__((unused)) void parse_int(parser_t *parser);

__attribute__((unused)) void parse_hex(parser_t *parser);

__attribute__((unused)) void parse_bin(parser_t *parser);

__attribute__((unused)) void parse_ident(parser_t *parser);

__attribute__((unused)) void parse_string(parser_t *parser);

__attribute__((unused)) void parse_literal(parser_t *parser);

__attribute__((unused)) void parse_unary_op(parser_t *parser);

__attribute__((unused)) void parse_binary_op(parser_t *parser);

__attribute__((unused)) void parse_parens(parser_t *parser);

__attribute__ ((unused)) void parse_block(parser_t *parser);

__attribute__((unused)) void parse_subscript(parser_t *parser);

void parse_expr_by_precedence(parser_t *parser, uint8_t min_preced);

parse_preced_rule_t preced_rules[] = {
    [TAG_INT] = {parse_int, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_HEX] = {parse_hex, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_BIN] = {parse_bin, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_TRUE] = {parse_literal, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_FALSE] = {parse_literal, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_STRING] = {parse_string, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_NIL] = {parse_literal, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_IDENT] = {parse_ident, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_LPAREN] = {parse_parens, NULL, PRECED_GROUPING, ASSOC_LEFT},
    [TAG_RPAREN] = {NULL, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_LSQUIGGLY] = {parse_block, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_RSQUIGGLY] = {NULL, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_LBRACKET] = {parse_subscript, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_RBRACKET] = {NULL, NULL, PRECED_NONE, ASSOC_LEFT},
    [TAG_MINUS] = {parse_unary_op, parse_binary_op, PRECED_TERM, ASSOC_LEFT},
    [TAG_PLUS] = {NULL, parse_binary_op, PRECED_TERM, ASSOC_LEFT},
    [TAG_TIMES] = {NULL, parse_binary_op, PRECED_FACTOR, ASSOC_LEFT},
    [TAG_DIVIDE]= {NULL, parse_binary_op, PRECED_FACTOR, ASSOC_LEFT},
    [TAG_BITWISE_OR] = {NULL, parse_binary_op, PRECED_BITWISE_OR, ASSOC_LEFT},
    [TAG_BITWISE_XOR] = {NULL, parse_binary_op, PRECED_BITWISE_XOR, ASSOC_LEFT},
    [TAG_BITWISE_AND] = {NULL, parse_binary_op, PRECED_BITWISE_AND, ASSOC_LEFT},
    [TAG_BITWISE_SHL] = {NULL, parse_binary_op, PRECED_BITWISE_SHIFT, ASSOC_LEFT},
    [TAG_BITWISE_SHR] = {NULL, parse_binary_op, PRECED_BITWISE_SHIFT, ASSOC_LEFT},
    [TAG_AND] = {NULL, parse_binary_op, PRECED_LOGICAL_AND, ASSOC_LEFT},
    [TAG_OR] = {NULL, parse_binary_op, PRECED_LOGICAL_OR, ASSOC_LEFT},
    [TAG_NOT] = {parse_unary_op, NULL, PRECED_TERM, ASSOC_LEFT},
    [TAG_GT] = {NULL, parse_binary_op, PRECED_CMP, ASSOC_LEFT},
    [TAG_LT] = {NULL, parse_binary_op, PRECED_CMP, ASSOC_LEFT},
    [TAG_GE] = {NULL, parse_binary_op, PRECED_CMP, ASSOC_LEFT},
    [TAG_LE] = {NULL, parse_binary_op, PRECED_CMP, ASSOC_LEFT},
    [TAG_EQ] = {NULL, parse_binary_op, PRECED_CMP, ASSOC_LEFT},
    [TAG_NE] = {NULL, parse_binary_op, PRECED_CMP, ASSOC_LEFT},
    [TAG_ASSIGN] = {NULL, parse_binary_op, PRECED_ASSIGN, ASSOC_RIGHT},
    [TAG_EOF]= {NULL, NULL, PRECED_EOF, ASSOC_LEFT},
};
