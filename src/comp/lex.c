#include <string.h>
#include <ctype.h>

#include "token.h"
#include "lex.h"

bool lexer_at_eof(lexer_t *lexer) {
    return *lexer->curr == '\0';
}

static char advance(lexer_t *lexer) {
    lexer->curr++;
    return lexer->curr[-1];
}

static char peek(lexer_t *lexer) {
    return *lexer->curr;
}

static char peek_next(lexer_t *lexer) {
    if (lexer_at_eof(lexer)) {
        return '\0';
    }
    return lexer->curr[1];
}

static bool match_next(lexer_t *lexer, const char ch) {
    if (lexer_at_eof(lexer)) {
        return false;
    }

    if (*lexer->curr != ch) {
        return false;
    }

    lexer->curr++;
    return true;
}

static void consume_ws(lexer_t *lexer) {
    for (;;) {
        char ch = peek(lexer);
        switch (ch) {
            case ' ':
            case '\t':
            case '\r':
                // Do not eat EOL.
                advance(lexer);
                break;

            default:
                return;
        }
    }
}

static token_t make_token(lexer_t *lexer, tag_t tag) {
    token_t token = {
        .tag = tag,
        .start = lexer->start,
        .len = (uint32_t) (lexer->curr - lexer->start)
    };
    /*
    char substr[lexer->curr - lexer->start + 1];
    memcpy(substr, lexer->start, token.len);
    substr[token.len] = '\0';
    printf("make_token: tag %d, text '%s'\n", tag, substr);
     */
    return token;
}

static token_t make_error_token(lexer_t *lexer, uint8_t err) {
    token_t token = make_token(lexer, TAG_ERROR);
    token.err = err;
    return token;
}

static token_t make_char_token(lexer_t *lexer) {
    // No content in char?
    if (peek(lexer) == '\'') {
        return make_error_token(lexer, LEX_ERR_INVALID_CHAR);
    }

    // Move past open quote.
    advance(lexer);

    // Quotes contain more than a single char?
    if (peek(lexer) != '\'') {
        return make_error_token(lexer, LEX_ERR_INVALID_CHAR);
    }

    // Move past close quote.
    advance(lexer);
    return make_token(lexer, TAG_CHAR);
}

static token_t make_string_token(lexer_t *lexer) {
    while (peek(lexer) != '"' && !lexer_at_eof(lexer)) {
        if (peek(lexer) == '\n') {
            lexer->curr_line_number++;
            lexer->curr_line_start = lexer->curr;
        }
        advance(lexer);
    }

    if (lexer_at_eof(lexer)) {
        return make_error_token(lexer, LEX_ERR_UNTERMINATED_STRING);
    }

    // Move past close quote.
    advance(lexer);

    return make_token(lexer, TAG_STRING);
}

static bool is_hex_digit(const char ch) {
    return ((ch >= '0' && ch <= '9') ||
            (ch >= 'A' && ch <= 'F') ||
            (ch >= 'a' && ch <= 'f')
    );
}

static bool is_bin_digit(const char ch) {
    return (ch == '0' || ch == '1');
}

static token_t lex_hex_num(lexer_t *lexer) {
    if (!is_hex_digit(peek(lexer))) {
        return make_error_token(lexer, LEX_ERR_INVALID_HEX_NUMERAL);
    }

    advance(lexer);

    while (is_hex_digit(peek(lexer))) {
        advance(lexer);
    }

    return make_token(lexer, TAG_HEX);
}

static token_t lex_bin_num(lexer_t *lexer) {
    if (!is_bin_digit(peek(lexer))) {
        return make_error_token(lexer, LEX_ERR_INVALID_BINARY_NUMERAL);
    }

    advance(lexer);

    while (is_bin_digit(peek(lexer))) {
        advance(lexer);
    }

    return make_token(lexer, TAG_BIN);
}

/**
 * Lex an int or float number.
 *
 * Float numbers do not need an initial decimal digit.
 * I.e., 0.123 and .123 are equally ok.
 */
static token_t lex_num(lexer_t *lexer) {
    if (peek(lexer) == 'x') {
        advance(lexer);
        return lex_hex_num(lexer);
    } else if (peek(lexer) == 'b') {
        advance(lexer);
        return lex_bin_num(lexer);
    }

    // Start of decimal or float.
    while (isdigit(peek(lexer))) {
        advance(lexer);
    }

    // Float?
    if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
        // Eat the '.'.
        advance(lexer);

        while (isdigit(peek(lexer))) {
            advance(lexer);
        }

        return make_token(lexer, TAG_FLOAT);
    }

    return make_token(lexer, TAG_INT);
}

static tag_t check_keyword(lexer_t *lexer, uint8_t start, uint8_t len, const char *expected, tag_t tag) {
    if (lexer->curr - lexer->start == start + len &&
        memcmp(lexer->start + start, expected, len) == 0) {
        return tag;
    }

    return TAG_IDENT;
}

static tag_t check_ident(lexer_t *lexer) {
    switch (lexer->start[0]) {
        case 'B':
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'o':
                        return check_keyword(lexer, 2, 5, "olean", TAG_TYPE_BOOLEAN);
                    case 'y': {
                        tag_t maybe_bytearray = check_keyword(lexer, 2, 7, "tearray", TAG_TYPE_BYTEARRAY);
                        if (maybe_bytearray == TAG_TYPE_BYTEARRAY) {
                            return maybe_bytearray;

                        }
                        return check_keyword(lexer, 2, 2, "te", TAG_TYPE_BYTE);
                    }
                }
            }
        case 'F':
            return check_keyword(lexer, 1, 4, "loat", TAG_TYPE_FLOAT);
        case 'I':
            return check_keyword(lexer, 1, 2, "nt", TAG_TYPE_INT);
        case 'L':
            return check_keyword(lexer, 1, 3, "ist", TAG_TYPE_LIST);
        case 'M':
            return check_keyword(lexer, 1, 2, "ap", TAG_TYPE_MAP);
        case 'S' :
            return check_keyword(lexer, 1, 5, "tring", TAG_TYPE_STRING);
        case 'a': {
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'b':
                        return check_keyword(lexer, 2, 1, "s", TAG_ABS);
                    case 'n':
                        return check_keyword(lexer, 2, 1, "d", TAG_AND);
                    case 'r':
                        return check_keyword(lexer, 2, 3, "ray", TAG_ARRAY);
                }
            }
            return check_keyword(lexer, 1, 1, "s", TAG_AS);
        }
        case 'b':
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'i':
                        return check_keyword(lexer, 2, 1, "n", TAG_BIN);
                    case 'r':
                        return check_keyword(lexer, 2, 3, "eak", TAG_BREAK);
                }
            }
        case 'c':
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'o': {
                        if (lexer->curr - lexer->start > 2) {
                            tag_t maybe_cos = check_keyword(lexer, 2, 1, "s", TAG_COS);
                            if (maybe_cos == TAG_COS) {
                                return maybe_cos;
                            }
                            return check_keyword(lexer, 2, 6, "ntinue", TAG_CONTINUE);
                        }
                    }
                }
            }
        case 'd':
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'a':
                        return check_keyword(lexer, 2, 2, "ta", TAG_DATA);
                    case 'e':
                        return check_keyword(lexer, 2, 1, "l", TAG_DEL);
                    case 'u':
                        return check_keyword(lexer, 2, 2, "mp", TAG_DUMP);
                }
                return check_keyword(lexer, 1, 1, "o", TAG_DO);
            }
        case 'e':
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'l':
                        return check_keyword(lexer, 2, 2, "se", TAG_ELSE);
                    case 'n':
                        return check_keyword(lexer, 2, 1, "v", TAG_ENV);
                    case 'x':
                        return check_keyword(lexer, 2, 1, "p", TAG_EXP);
                }
            }
        case 'f':
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'a':
                        return check_keyword(lexer, 2, 3, "lse", TAG_FALSE);
                    case 'o':
                        return check_keyword(lexer, 2, 1, "r", TAG_FOR);
                    case 'u':
                        return check_keyword(lexer, 2, 1, "n", TAG_FUNC_DEF);
                }
            }
        case 'h':
            if (lexer->curr - lexer->start > 1) {
                return check_keyword(lexer, 1, 2, "ex", TAG_HEX);
            }
        case 'i':
            if (lexer->curr - lexer->start > 1) {
                tag_t maybe = check_keyword(lexer, 1, 1, "f", TAG_IF);
                if (maybe == TAG_IF) {
                    return maybe;
                }
                maybe = check_keyword(lexer, 1, 4, "nput", TAG_INPUT);
                if (maybe == TAG_INPUT) {
                    return maybe;
                }
                maybe = check_keyword(lexer, 1, 1, "s", TAG_IS);
                if (maybe == TAG_IS) {
                    return maybe;
                }
                return check_keyword(lexer, 1, 1, "n", TAG_IN);
            }
        case 'l':
            if (lexer->curr - lexer->start > 1) {
                tag_t maybe = check_keyword(lexer, 1, 1, "n", TAG_LN);
                if (maybe == TAG_LN) {
                    return maybe;
                }
                return check_keyword(lexer, 1, 2, "og", TAG_LOG);
            }
        case 'm':
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'a':
                        return check_keyword(lexer, 2, 3, "tch", TAG_MATCH);
                    case 'e':
                        return check_keyword(lexer, 2, 1, "m", TAG_MEM);
                }

            }
        case 'n':
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'i':
                        return check_keyword(lexer, 2, 1, "l", TAG_NIL);
                    case 'o':
                        return check_keyword(lexer, 2, 1, "t", TAG_NOT);
                }
            }
        case 'o':
            if (lexer->curr - lexer->start > 1) {
                return check_keyword(lexer, 1, 1, "r", TAG_OR);
            }
        case 'p':
            if (lexer->curr - lexer->start > 1) {
                return check_keyword(lexer, 1, 4, "rint", TAG_PRINT);
            }
        case 'r':
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'a':
                        return check_keyword(lexer, 2, 2, "nd", TAG_RAND);
                    case 'e':
                        if (lexer->curr - lexer->start > 2) {
                            switch (lexer->start[2]) {
                                case 'a':
                                    return check_keyword(lexer, 3, 1, "d", TAG_READ);
                                case 't':
                                    return check_keyword(lexer, 3, 3, "urn", TAG_FUNC_RETURN);
                            }
                        }
                }
            }
        case 's':
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'i':
                        return check_keyword(lexer, 2, 1, "n", TAG_SIN);
                    case 'q':
                        return check_keyword(lexer, 2, 2, "rt", TAG_SQRT);
                    case 't':
                        if (lexer->curr - lexer->start > 2) {
                            switch (lexer->start[2]) {
                                case 'e':
                                    return check_keyword(lexer, 3, 1, "p", TAG_STEP);
                                case 'r':
                                    return check_keyword(lexer, 3, 3, "uct", TAG_TYPEDEF_STRUCT);

                            }
                        }
                }
            }
        case 't':
            if (lexer->curr - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'a':
                        return check_keyword(lexer, 2, 1, "n", TAG_TAN);
                    case 'h':
                        return check_keyword(lexer, 2, 2, "en", TAG_THEN);
                    case 'r':
                        return check_keyword(lexer, 2, 2, "ue", TAG_TRUE);
                    case 'y':
                        if (lexer->curr - lexer->start > 2) {
                            tag_t maybe = check_keyword(lexer, 2, 4, "peof", TAG_TYPEOF);
                            if (maybe == TAG_TYPEOF) {
                                return maybe;
                            }
                            return check_keyword(lexer, 2, 2, "pe", TAG_TYPE);
                        }
                }
            }
        case 'v':
            if (lexer->curr - lexer->start > 2) {
                tag_t maybe = check_keyword(lexer, 1, 2, "al", TAG_INVARIABLE);
                if (maybe == TAG_INVARIABLE) {
                    return maybe;
                }
                return check_keyword(lexer, 1, 2, "ar", TAG_VARIABLE);
            }
        case 'w':
            if (lexer->curr - lexer->start > 1) {
                return check_keyword(lexer, 1, 4, "hile", TAG_WHILE);
            }

    }

    return TAG_IDENT;


}

static token_t lex_identifier(lexer_t *lexer) {
    while (isalnum(peek(lexer))) {
        advance(lexer);
    }

    return make_token(lexer, check_ident(lexer));
}

static token_t lex_field_or_method_access(lexer_t *lexer) {
    while (isalnum(peek(lexer))) {
        advance(lexer);
    }

    if (peek(lexer) == '(') {
        return make_token(lexer, TAG_METHOD_CALL);
    }

    advance(lexer);
    return make_token(lexer, TAG_FIELD_ACCESS);
}

token_t next_token(lexer_t *lexer) {
    consume_ws(lexer);
    lexer->start = lexer->curr;

    if (*lexer->curr == '\0') {
        return make_token(lexer, TAG_EOF);
    }

    char ch = advance(lexer);

    if (ch == '\n') {
        return make_token(lexer, TAG_EOL);
    }

    if (ch >= '0' && ch <= '9') {
        return lex_num(lexer);
    }

    if ((ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
        (ch == '_')) {
        return lex_identifier(lexer);
    }

    switch (ch) {
        case '{':
            return make_token(lexer, TAG_LSQUIGGLY);
        case '}':
            return make_token(lexer, TAG_RSQUIGGLY);
        case '(':
            return make_token(lexer, TAG_LPAREN);
        case ')':
            return make_token(lexer, TAG_RPAREN);
        case '[':
            return make_token(lexer, TAG_LBRACKET);
        case ']':
            return make_token(lexer, TAG_RBRACKET);
        case ',':
            return make_token(lexer, TAG_COMMA);
        case ':':
            return make_token(lexer, TAG_COLON);
        case '+':
            return make_token(lexer, TAG_PLUS);
            // The parser can treat this as the sign op or a binop as context dictates.
        case '-':
            return make_token(lexer, match_next(lexer, '>') ? TAG_MAPS_TO : TAG_MINUS);
        case '*':
            return make_token(lexer, TAG_TIMES);
        case '/': {
            if (match_next(lexer, '/')) {
                // Ignore comment.
                while (peek(lexer) != '\n' && peek(lexer) != '\0') {
                    advance(lexer);
                }
                return make_token(lexer, TAG_EOL);
            } else {
                return make_token(lexer, TAG_DIVIDE);
            }
        }
        case '%':
            return make_token(lexer, TAG_MOD);
        case '&':
            return make_token(lexer, TAG_BITWISE_AND);
        case '|':
            return make_token(lexer, TAG_BITWISE_OR);
        case '^':
            return make_token(lexer, TAG_BITWISE_XOR);
        case '~':
            return make_token(lexer, TAG_BITWISE_NOT);
        case '.': {
            if (match_next(lexer, '.')) {
                return make_token(lexer, TAG_RANGE);
            }

            // Start of a float number?
            if (peek(lexer) >= '0' && peek(lexer) <= '9') {
                return lex_num(lexer);
            }

            // Otherwise start of a field or method access.
            return lex_field_or_method_access(lexer);
        }
        case '<': {
            if (match_next(lexer, '=')) {
                return make_token(lexer, TAG_LE);
            }
            if (match_next(lexer, '<')) {
                return make_token(lexer, TAG_BITWISE_SHL);
            }
            return make_token(lexer, TAG_LT);
        }
        case '>': {
            if (match_next(lexer, '=')) {
                return make_token(lexer, TAG_GE);
            }
            if (match_next(lexer, '>')) {
                return make_token(lexer, TAG_BITWISE_SHR);
            }
            return make_token(lexer, TAG_GT);
        }
        case '=':
            return make_token(lexer, match_next(lexer, '=') ? TAG_EQ : TAG_ASSIGN);
        case '!':
            return make_token(lexer, match_next(lexer, '=') ? TAG_NE : TAG_NOT);
        case '\'':
            return make_char_token(lexer);
        case '"':
            return make_string_token(lexer);
        default:
            return make_error_token(lexer, LEX_ERR_INVALID_SEQUENCE);
    }
}

void lexer_init(lexer_t *lexer, const char input[]) {
    lexer->err_pos = 0;
    lexer->curr_line_number = 1;
    lexer->curr_line_start = input;
    lexer->start = input;
    lexer->curr = input;
}
