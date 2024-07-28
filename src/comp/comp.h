#pragma once

#include "cg.h"
#include "token.h"
#include "lex.h"

static const uint8_t magic[] = {'E', 'T', 'H', 'L'};
static const uint8_t major = 0;
static const uint8_t minor = 1;

#define BYTEARRAY_MAX (1024)
#define MIN_BYTECODE_ALLOC (32)

typedef enum {
    CONST_BYTE,
    CONST_INT,
    CONST_FLOAT32,
    CONST_BYTEARRAY,
    CONST_STRING,
} const_type_t;

typedef struct {
    const_type_t type;
    uint8_t length;
    uint8_t data[];
} const_info_t;

typedef enum {
    COMP_ERR_NO_ERROR,
    COMP_ERR_UNEXPECTED_TOKEN,
    COMP_ERR_UNHANDLED_LITERAL,
    COMP_ERR_UNHANDLED_PREFIX_OP,
    COMP_ERR_UNHANDLED_INFIX_OP,
    COMP_ERR_EXPECTED_EXPRESSION,
    COMP_ERR_TOO_MANY_CONSTANTS,
    COMP_ERR_TOO_MANY_ELEMENTS,
    COMP_ERR_NON_BYTE_IN_BYTEARRAY,
    COMP_ERR_INSUFFICIENT_SPACE_FOR_BYTECODE,
} comp_err_t;

static const char* comp_err_names[] = {
    [COMP_ERR_NO_ERROR] = "Compiled OK",
    [COMP_ERR_UNEXPECTED_TOKEN] = "Unexpected token",
    [COMP_ERR_UNHANDLED_LITERAL] = "Unhandled literal",
    [COMP_ERR_UNHANDLED_PREFIX_OP] = "Unhandled prefix operator",
    [COMP_ERR_UNHANDLED_INFIX_OP] = "Unhandled infix operator",
    [COMP_ERR_EXPECTED_EXPRESSION] = "Expected an expression",
    [COMP_ERR_TOO_MANY_CONSTANTS] = "Too many constants",
    [COMP_ERR_TOO_MANY_ELEMENTS] = "Too many elements",
    [COMP_ERR_NON_BYTE_IN_BYTEARRAY] = "Byte array value larger than one byte",
    [COMP_ERR_INSUFFICIENT_SPACE_FOR_BYTECODE] = "Insufficient space for bytecode",
};

typedef struct parser_t {
    lexer_t *lexer;
    cg_t *cg;
    token_t prev;
    token_t curr;
    uint8_t err;
} parser_t;

typedef struct comp_t {

} comp_t;

comp_err_t codegen(const char *input, cg_t *cg);

comp_err_t compile(const cg_t *cg, uint32_t max_size, uint8_t buf[], uint32_t *size);
