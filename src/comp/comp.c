#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../common/op.h"
#include "../common/ptr.h"
#include "cg.h"
#include "comp.h"
#include "dis.h"
#include "lex.h"
#include "preced.h"
#include "val.h"

static void parse_expr(parser_t *parser);

static void parse_stmt(parser_t *parser);

static void parse_decl(parser_t *parser);

static void error(parser_t *parser, comp_err_t which) {
    printf("ERROR: Failed to parse token: error %d\n", which);
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
        printf("Eat unexpected token %d\n", parser->curr.tag);
        error(parser, COMP_UNEXPECTED_TOKEN);
        return;
    }

    advance(parser);
}

static bool check_token_tag(parser_t *parser, tag_t tag) {
    return parser->curr.tag == tag;
}

static bool token_tag_matches(parser_t *parser, tag_t tag) {
    if (!check_token_tag(parser, tag)) {
        return false;
    }

    advance(parser);
    return true;
}

static void emit_byte(parser_t *parser, uint8_t byte) {
    /*
    if (byte < 0xa5) {
        printf("emit %02x %s\n", byte, op_names[byte]);
    } else {
        printf("emit %02x\n", byte);
    }
     */

    cg_byte(parser->cg, byte);
}

static void emit_bytes(parser_t *parser, uint8_t byte1, uint8_t byte2) {
    emit_byte(parser, byte1);
    emit_byte(parser, byte2);
}

static void emit_const_bool(parser_t *parser, bool val) {
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
        val_t v = {.type = VAL_TYPE_INT, .as.intval = val};
        uint8_t k;
        err = cg_put_const(parser->cg, v, &k);
        emit_bytes(parser, VM_OP_ICONST, k);
    }

    return err;
}

static map_err_t emit_const_obj_str(parser_t *parser, const char *chars, uint32_t length) {
    map_err_t err;
    obj_str_t *obj_str = obj_str_new(chars, length);

    val_t v = {
        .type = VAL_TYPE_OBJ,
        .as.objval = (obj_t *) obj_str,
    };
    uint8_t k;
    err = cg_put_const(parser->cg, v, &k);
    emit_bytes(parser, VM_OP_SCONST, k);

    return err;
}

static uint16_t emit_jump(parser_t *parser, vm_op_t jump_op) {
    emit_byte(parser, jump_op);
    uint32_t loc = parser->cg->len;

    // Placeholder for 16-bit jump address.
    emit_byte(parser, 0xa5);
    emit_byte(parser, 0xa5);

    // Return address of jump address bytes.
    return loc;
}

static void set_jump_addr(parser_t *parser, uint16_t from_addr, uint16_t to_addr) {
    // TODO do we want relative jumps? Save a byte?
    uint8_t low_byte = (uint8_t) (to_addr & 0xff);
    uint8_t high_byte = (uint8_t) ((to_addr & 0xff00) >> 8);

    parser->cg->bytecode[from_addr] = low_byte;
    parser->cg->bytecode[from_addr + 1] = high_byte;
}

void parse_expr_by_precedence(parser_t *parser, uint8_t min_preced) {
    advance(parser);
    tag_t tag = parser->prev.tag;

    if (tag == TAG_EOL) {
        if (min_preced <= PRECED_NONE) {
            return;
        } else if (tag == TAG_EOL) {
            // Inside an expression, consume whitespace.
            // This lets us parse, for example, multi-line parentheses.
            advance(parser);
            tag = parser->prev.tag;
        }
    }

    parse_func prefix_rule = preced_rules[tag].parse_prefix;

    if (prefix_rule == NULL) {
        printf("No prefix rule for token %s\n", tag_names[tag]);
        error(parser, COMP_EXPECTED_EXPRESSION);
        return;
    }

    prefix_rule(parser);

    // Pratt precedence climbing.
    while (min_preced <= preced_rules[parser->curr.tag].precedence) {
        advance(parser);
        parse_func infix_rule = preced_rules[parser->prev.tag].parse_infix;
        if (infix_rule != NULL) {
            infix_rule(parser);
        }
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
    (void) emit_const_obj_str(parser, parser->prev.start, parser->prev.len);
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_string(parser_t *parser) {
    // Remove surrounding quotes.
    if (*parser->prev.start != '"') {
        error(parser, COMP_UNEXPECTED_TOKEN);
        return;
    }

    const char *start = parser->prev.start + 1;
    uint32_t len = parser->prev.len - 2;
    (void) emit_const_obj_str(parser, start, len);
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_literal(parser_t *parser) {
    switch (parser->prev.tag) {
        case TAG_TRUE:
            emit_byte(parser, VM_OP_ZPUSH_T);
            break;
        case TAG_FALSE:
            emit_byte(parser, VM_OP_ZPUSH_F);
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
        case TAG_GT:
            emit_byte(parser, VM_OP_GT);
            break;
        case TAG_LT:
            emit_byte(parser, VM_OP_LT);
            break;
        case TAG_GE:
            emit_byte(parser, VM_OP_GE);
            break;
        case TAG_LE:
            emit_byte(parser, VM_OP_LE);
            break;
        case TAG_EQ:
            emit_byte(parser, VM_OP_EQ);
            break;
        case TAG_NE:
            emit_byte(parser, VM_OP_NE);
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
    while (!check_token_tag(parser, TAG_RPAREN) &&
           !check_token_tag(parser, TAG_EOF)) {
        parse_decl(parser);
        token_tag_matches(parser, TAG_EOL);
    }
    eat(parser, TAG_RPAREN);
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_subscript(parser_t *parser) {
    while (!check_token_tag(parser, TAG_RBRACKET) &&
           !check_token_tag(parser, TAG_EOF)) {
        parse_expr(parser);
        token_tag_matches(parser, TAG_EOL);
    }
    emit_byte(parser, VM_OP_ALOAD);
    eat(parser, TAG_RBRACKET);
}

static void parse_expr(parser_t *parser) {
    parse_expr_by_precedence(parser, PRECED_ASSIGN);
}

static void parse_if_stmt(parser_t *parser) {
    uint16_t from_if_addr;
    uint16_t from_else_addr;

    eat(parser, TAG_LPAREN);
    parse_parens(parser);

    eat(parser, TAG_THEN);

    from_if_addr = emit_jump(parser, VM_OP_JZ);

    parse_stmt(parser);

    if (token_tag_matches(parser, TAG_ELSE)) {
        from_else_addr = emit_jump(parser, VM_OP_JMP);

        set_jump_addr(parser, from_if_addr, parser->cg->len);

        // Could be `else if (decl)` or `else (stmt)`
        // so handle all cases with decl.
        parse_decl(parser);

        set_jump_addr(parser, from_else_addr, parser->cg->len);
    } else {
        set_jump_addr(parser, from_if_addr, parser->cg->len);
    }
}


// Referenced via pointer in the precedence table.
__attribute__ ((unused)) void parse_block(parser_t *parser) {
    while (!token_tag_matches(parser, TAG_RSQUIGGLY) &&
           !token_tag_matches(parser, TAG_EOF)) {
        parse_decl(parser);
    }
}


static void enter_scope(parser_t *parser) {
    // compiler enter scope
}

static void exit_scope(parser_t *parser) {
    // compiler exit scope
}

/*
 * Parse stmt
 *
 * stmt -> if_stmt
 *       | expr
 *       | block
 *
 * block -> "{" decl* "}"
 */
static void parse_stmt(parser_t *parser) {
    if (token_tag_matches(parser, TAG_IF)) {
        parse_if_stmt(parser);
    } else {
        parse_expr(parser);
    }
}

/*
 * Parse declaration
 *
 * decl -> stmt
 */
static void parse_decl(parser_t *parser) {
    parse_stmt(parser);
    token_tag_matches(parser, TAG_EOL);
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
    while (!token_tag_matches(&parser, TAG_EOF)) {
        parse_decl(&parser);
    }
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
        val_t k = {.type = VAL_TYPE_INT, .as.intval=i};
        val_t *v = map_get(cg->consts, &k);

        switch (v->type) {
            case VAL_TYPE_INT: {
                // Ints are packed into as few bytes as possible,
                // least-significant byte first.
                buf[offset++] = CONST_INT;
                uint32_t byte_len = offset++; // Set value below.
                uint8_t intlen;
                int intval = v->as.intval;
                if (intval >= -128 && intval <= 127) {
                    buf[offset++] = (uint8_t) (v->as.intval & 0x000000ff);
                    intlen = 1;
                }
                if (intval >= -32768 && intval <= 32767) {
                    buf[offset++] = (uint8_t) ((v->as.intval & 0x0000ff00) >> 8);
                    intlen = 2;
                }
                if (intval >= -8388608 && intval <= 8388607) {
                    buf[offset++] = (uint8_t) ((v->as.intval & 0x00ff0000) >> 16);
                    intlen = 3;
                }
                if (intval) {
                    buf[offset++] = (uint8_t) (((uint32_t) v->as.intval & 0xff000000) >> 24);
                    intlen = 4;
                }
                buf[byte_len] = intlen;
                break;
            }
            case VAL_TYPE_OBJ:
                // TODO check string too long
                buf[offset++] = CONST_STRING;
                obj_str_t *obj_str = AS_OBJ_STR(v);
                uint8_t slen = buf[offset++] = obj_str->length;
                // Deliberately not null-terminated
                mem_cp(buf + offset, AS_OBJ_STR(v)->chars, slen);
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
