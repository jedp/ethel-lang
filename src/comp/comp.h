#pragma once

#include "err.h"
#include "cg.h"
#include "token.h"
#include "lex.h"

static const uint8_t magic[] = {'E', 'T', 'H', 'L'};
static const uint8_t major = 0;
static const uint8_t minor = 1;

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
    COMP_UNEXPECTED_TOKEN,
    COMP_UNHANDLED_LITERAL,
    COMP_UNHANDLED_PREFIX_OP,
    COMP_UNHANDLED_INFIX_OP,
    COMP_EXPECTED_EXPRESSION,
    COMP_TOO_MANY_CONSTANTS,
    COMP_UNEXPECTED_CONST_TYPE,
    COMP_INSUFFICIENT_SPACE_FOR_BYTECODE,
} comp_err_t;

typedef struct parser_t {
    lexer_t *lexer;
    cg_t *cg;
    token_t prev;
    token_t curr;
    uint8_t err;
} parser_t;

typedef struct comp_t {

} comp_t;

error_t codegen(const char *input, cg_t *cg);

error_t compile(const cg_t *cg, uint32_t max_size, uint8_t buf[], uint32_t *size);
