#include <stdlib.h>
#include <string.h>
#include "../common/op.h"
#include "../common/ptr.h"
#include "cg.h"
#include "comp.h"
#include "dis.h"
#include "lex.h"
#include "preced.h"

static void parse_expr(parser_t *parser);

static void error(parser_t *parser, uint8_t which) {
    printf("ERROR: Failed to parse token %d\n", which);
    parser->err = which;
}

/*
 * Read ahead one token, skipping errors.
 */
static void advance(parser_t *parser) {
    parser->prev = parser->curr;

    while (1) {
        parser->curr = next_token(parser->lexer);
        if (parser->curr.tag != TAG_ERROR) {
            break;
        }
    }
}

static void eat(parser_t *parser, tag_t tag) {
    if (parser->curr.tag != tag) {
        error(parser, COMP_UNEXPECTED_TOKEN);
        return;
    }

    advance(parser);
}

static void emit_byte(parser_t *parser, uint8_t byte) {
    cg_byte(parser->cg, byte);
}

static void emit_bytes(parser_t *parser, uint8_t byte1, uint8_t byte2) {
    emit_byte(parser, byte1);
    emit_byte(parser, byte2);
}

static void emit_const_bool(parser_t *parser, boolean val) {
    emit_byte(parser, val ? VM_OP_ZPUSH_T : VM_OP_ZPUSH_F);
}

static map_err_t emit_const_int(parser_t *parser, int val) {
    map_err_t err = MAP_OK;

    if (val == -1) {
        emit_byte(parser, VM_OP_IPUSH_1N);
    } else if (val == 0) {
        emit_byte(parser, VM_OP_IPUSH_0);
    } else if (val == 1) {
        emit_byte(parser, VM_OP_IPUSH_1);
    } else if (val >= -128 && val <= 127) {
        emit_bytes(parser, VM_OP_IPUSH, (uint8_t) val & 0xff);
    } else {
        map_elem_t v = {.type = MAP_ELEM_INT_TYPE, .elem.intval = val};
        uint8_t k;
        err = cg_put_const(parser->cg, v, &k);
        emit_bytes(parser, VM_OP_ICONST, k);
    }

    return err;
}

static map_err_t emit_const_str(parser_t *parser, const char *val, uint32_t len) {
    map_err_t err;

    char *strval = malloc(len + 1);
    strncpy(strval, val, len);
    strval[len] = '\0';

    map_elem_t v = {
        .type = MAP_ELEM_STRING_TYPE,
        .elem.stringval_ptr = strval,
    };
    uint8_t k;
    err = cg_put_const(parser->cg, v, &k);
    emit_bytes(parser, VM_OP_SCONST, k);

    return err;
}

void parse_expr_by_precedence(parser_t *parser, uint8_t min_preced) {
    advance(parser);
    tag_t tag = parser->prev.tag;
    parse_func prefix_rule = preced_rules[tag].parse_prefix;

    if (prefix_rule == NULL) {
        error(parser, COMP_EXPECTED_EXPRESSION);
        return;
    }

    prefix_rule(parser);

    // Pratt precedence climbing.
    while (min_preced <= preced_rules[parser->curr.tag].precedence) {
        advance(parser);
        parse_func infix_rule = preced_rules[parser->prev.tag].parse_infix;
        if (infix_rule != NULL)
            infix_rule(parser);
    }
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_int(parser_t *parser) {
    int const_int = (int) strtol(parser->prev.start, NULL, 10);
    (void) emit_const_int(parser, const_int);
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_hex(parser_t *parser) {
    int const_int = (int) strtol(parser->prev.start, NULL, 16);
    (void) emit_const_int(parser, const_int);
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_bin(parser_t *parser) {
    // strtol removes the '0x' for hex, but not the '0b' for bin.
    int const_int = (int) strtol(parser->prev.start + 2, NULL, 2);
    (void) emit_const_int(parser, const_int);
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_ident(parser_t *parser) {
    (void) emit_const_str(parser, parser->prev.start, parser->prev.len);
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_literal(parser_t *parser) {
    switch (parser->prev.tag) {
        case TAG_TRUE:
            emit_byte(parser, VM_OP_TRUE);
            break;
        case TAG_FALSE:
            emit_byte(parser, VM_OP_FALSE);
            break;
        case TAG_NIL:
            emit_byte(parser, VM_OP_NIL);
            break;
        default:
            error(parser, COMP_UNHANDLED_LITERAL);
    }
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_unary_op(parser_t *parser) {
    tag_t op = parser->prev.tag;

    parse_expr_by_precedence(parser, PRECED_UNARY);

    switch (op) {
        case TAG_MINUS:
            emit_byte(parser, VM_OP_NEG);
            break;
        case TAG_NOT:
            emit_byte(parser, VM_OP_LOGICAL_NOT);
            break;
        default:
            error(parser, COMP_UNHANDLED_PREFIX_OP);
    }
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_binary_op(parser_t *parser) {
    tag_t op = parser->prev.tag;

    // Parse and push the remainder of the expression.
    parse_preced_rule_t op_rule = preced_rules[op];
    parse_expr_by_precedence(parser, (uint8_t) op_rule.precedence + (uint8_t) op_rule.associativity);

    // Push the operator last.
    switch (op) {
        case TAG_PLUS:
            emit_byte(parser, VM_OP_ADD);
            break;
        case TAG_MINUS:
            emit_byte(parser, VM_OP_SUB);
            break;
        case TAG_TIMES:
            emit_byte(parser, VM_OP_MUL);
            break;
        case TAG_DIVIDE:
            emit_byte(parser, VM_OP_DIV);
            break;
        case TAG_BITWISE_OR:
            emit_byte(parser, VM_OP_BIN_OR);
            break;
        case TAG_BITWISE_XOR:
            emit_byte(parser, VM_OP_BIN_XOR);
            break;
        case TAG_BITWISE_AND:
            emit_byte(parser, VM_OP_BIN_AND);
            break;
        case TAG_BITWISE_SHL:
            emit_byte(parser, VM_OP_BIN_SHL);
            break;
        case TAG_BITWISE_SHR:
            emit_byte(parser, VM_OP_BIN_SHR);
            break;
        case TAG_AND:
            emit_byte(parser, VM_OP_LOGICAL_AND);
            break;
        case TAG_OR:
            emit_byte(parser, VM_OP_LOGICAL_OR);
            break;
        case TAG_ASSIGN:
            emit_byte(parser, VM_OP_ASSIGN);
            break;
        default:
            error(parser, COMP_UNHANDLED_INFIX_OP);
            break;
    }
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_parens(parser_t *parser) {
    parse_expr(parser);
    printf("TODO: TAG_RPAREN eaten by precedence climbing. Should it be?\n");
//    eat(parser, TAG_RPAREN);
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_subscript(parser_t *parser) {
    (void) parser;
}

static void parse_expr(parser_t *parser) {
    parse_expr_by_precedence(parser, PRECED_NONE);
}

error_t codegen(const char *input, cg_t *cg) {
    printf("Compiling input: %s\n", input);
    lexer_t lexer;
    lexer_init(&lexer, input);

    parser_t parser;
    parser.lexer = &lexer;
    parser.cg = cg;
    parser.err = COMP_ERR_NO_ERROR;

    advance(&parser);
    parse_expr(&parser);
    eat(&parser, TAG_EOF);
    emit_byte(&parser, VM_OP_RET);

    print_dis(parser.cg);

    return parser.err;
}

error_t compile(const cg_t *cg, uint32_t max_size, uint8_t buf[], uint32_t *size) {
    if (max_size < MIN_BYTECODE_ALLOC) {
        return COMP_INSUFFICIENT_SPACE_FOR_BYTECODE;
    }

    uint32_t offset;

    // Header
    memcpy(buf, magic, 4);
    offset = 4;
    buf[offset++] = major;
    buf[offset++] = minor;

    // Const pool
    if (cg->consts->buckets->nelems > UINT8_MAX) {
        return COMP_TOO_MANY_CONSTANTS;
    }
    buf[offset++] = (uint8_t) cg->consts->buckets->nelems;
    for (uint8_t i = 1; i <= (uint8_t) cg->consts->buckets->nelems; i++) {
        map_elem_t k = {.type = MAP_ELEM_INT_TYPE, .elem.intval=i};
        map_elem_t *v = map_get(cg->consts, &k);

        switch (v->type) {
            case MAP_ELEM_INT_TYPE: {
                // Ints are packed into as few bytes as possible,
                // least-significant byte first.
                buf[offset++] = CONST_INT;
                uint32_t byte_len = offset++; // Set value below.
                uint8_t intlen;
                int intval = v->elem.intval;
                if (intval >= -128 && intval <= 127) {
                    buf[offset++] = (uint8_t) (v->elem.intval & 0x000000ff);
                    intlen = 1;
                }
                if (intval >= -32768 && intval <= 32767) {
                    buf[offset++] = (uint8_t) ((v->elem.intval & 0x0000ff00) >> 8);
                    intlen = 2;
                }
                if (intval >= -8388608 && intval <= 8388607) {
                    buf[offset++] = (uint8_t) ((v->elem.intval & 0x00ff0000) >> 16);
                    intlen = 3;
                }
                if (intval) {
                    buf[offset++] = (uint8_t) (((uint32_t) v->elem.intval & 0xff000000) >> 24);
                    intlen = 4;
                }
                buf[byte_len] = intlen;
                break;
            }
            case MAP_ELEM_STRING_TYPE:
                // TODO check string too long
                buf[offset++] = CONST_STRING;
                uint8_t slen = buf[offset++] = (uint8_t) strlen(v->elem.stringval_ptr);
                // Deliberately not null-terminated
                mem_cp(buf + offset, v->elem.stringval_ptr, slen);
                offset += slen;
                break;
            default:
                return COMP_UNEXPECTED_CONST_TYPE;

        }
    }

    *size = offset;
    /*
    for (uint8_t i = 0; i < offset; i++) {
        printf("buf[%d]\t%x\n", i, buf[i]);
    }
     */

    return ERR_NO_ERROR;
}
