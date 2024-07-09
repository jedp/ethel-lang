#pragma once

#include <inttypes.h>
#include "../common/err.h"
#include "../common/def.h"
#include "token.h"

typedef enum lex_err {
    LEX_ERR_NO_ERROR,
    LEX_ERR_INVALID_SEQUENCE,
    LEX_ERR_UNTERMINATED_STRING,
    LEX_ERR_INVALID_CHAR,
    LEX_ERR_INVALID_HEX_NUMERAL,
    LEX_ERR_INVALID_BINARY_NUMERAL,
} lex_err_t;

typedef struct {
    uint32_t err_pos;
    uint32_t line_pos;
    uint32_t char_pos;
    const char *start;
    const char *curr;
} lexer_t;

void lexer_init(lexer_t *lexer, const char input[]);

boolean lexer_at_eof(lexer_t *lexer);

token_t next_token(lexer_t *lexer);
