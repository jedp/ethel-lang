#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include "../mem/mem.h"
#include "../common/op.h"
#include "../common/ptr.h"
#include "cg.h"
#include "comp.h"
#include "dis.h"
#include "lex.h"
#include "preced.h"
#include "val.h"


static uint32_t obj_arr_append(obj_arr_t *obj_arr, uint8_t byte) {
    if (obj_arr->buflen < obj_arr->length + 1) {
        obj_arr->buflen = ARR_GROW_MAX_SIZE(obj_arr->buflen);
        obj_arr->buf = ARR_GROW(uint8_t, obj_arr->buf, obj_arr->buflen);

        if (obj_arr->buf == NULL) {
            printf("Realloc failed!\n");
            exit(1);
        }
    }

    obj_arr->buf[obj_arr->length] = byte;
    obj_arr->length++;

    return obj_arr->length;
}

static void parse_expr(parser_t *parser);

static void parse_stmt(parser_t *parser);

static void parse_decl(parser_t *parser);

static void parser_error(parser_t *parser, comp_err_t which, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fflush(stdout);
    fflush(stderr);

    fprintf(stderr, "ERROR: %s\n", comp_err_names[which]);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);

    // Make it 1-indexed for legibility.
    int char_pos = (int) (&parser->curr.start + 1 - &parser->lexer->curr_line_start);
    fprintf(stderr, "Input line %d, column %d:\n", parser->lexer->curr_line_number, char_pos);

    // Print the line context.
    if (char_pos > 60) {
        fprintf(stderr, parser->lexer->curr_line_start, 60);
        fprintf(stderr, " ...\n");
    } else {
        char buf[60] = {0};
        buf[59] = '\0';
        int offset = 0;
        for (;;) {
            buf[offset] = (char) *(parser->lexer->curr_line_start + offset);
            if (offset >= 59
                || buf[offset] == '\n'
                || buf[offset] == '\0'
                || buf[offset] == EOF) {
                break;
            }
            offset++;
        }
        buf[offset] = '\0';
        fprintf(stderr, buf, 60);
        fputc('\n', stderr);

        // Point to the location of the error.
        for (int i = 0; i < char_pos; i++) {
            fprintf(stderr, "-");
        }
        fprintf(stderr, "^\n");
    }


    fflush(stderr);
    va_end(args);

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
        parser_error(parser, COMP_ERR_UNEXPECTED_TOKEN,
                     "Expected to eat %s, but ate %s\n", tag_names[tag], tag_names[parser->curr.tag]);
        return;
    }

    advance(parser);
}

static bool check_token_tag(parser_t *parser, tag_t tag) {
    return parser->curr.tag == tag;
}

static bool token_tag_match_and_consume(parser_t *parser, tag_t tag) {
    if (!check_token_tag(parser, tag)) {
        return false;
    }

    advance(parser);
    return true;
}

static void emit_byte(parser_t *parser, uint8_t byte) {
    /*
    if (byte < VM_OP_MAX) {
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

static void emit_const_byte(parser_t *parser, uint32_t byte) {
    emit_byte(parser, VM_OP_BPUSH);
    emit_byte(parser, byte);
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

static map_err_t emit_const_obj_arr(parser_t *parser, obj_arr_t *obj_arr) {
    map_err_t err;

    val_t v = {
        .type = VAL_TYPE_OBJ,
        .as.objval = (obj_t *) obj_arr,
    };
    uint8_t k;
    err = cg_put_const(parser->cg, v, &k);
    emit_bytes(parser, VM_OP_ACONST, k);

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
        parser_error(parser, COMP_ERR_EXPECTED_EXPRESSION,
                     "Expected valid expression before %s\n", tag_names[tag]);
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
__attribute__((unused)) void parse_char(parser_t *parser) {
    // Remove surrounding quotes.
    if (*parser->prev.start != '\'') {
        parser_error(parser, COMP_ERR_UNEXPECTED_TOKEN,
                     "Expected single quote; got char 0x%02x", *parser->prev.start);
        return;
    }

    const char *ch = parser->prev.start + 1;

    (void) emit_const_byte(parser, (uint8_t) *ch);
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_string(parser_t *parser) {
    // Remove surrounding quotes.
    if (*parser->prev.start != '"') {
        parser_error(parser, COMP_ERR_UNEXPECTED_TOKEN,
                     "Expected double quote; got char 0x%02x", *parser->prev.start);
        return;
    }

    const char *start = parser->prev.start + 1;
    uint32_t len = parser->prev.len - 2;
    (void) emit_const_obj_str(parser, start, len);
}

static void parse_array_alloc(parser_t *parser) {
    while (!check_token_tag(parser, TAG_RPAREN) &&
           !check_token_tag(parser, TAG_EOF)) {
        // Push uint32 size of array.
        parse_decl(parser);
        token_tag_match_and_consume(parser, TAG_EOL);
    }
    eat(parser, TAG_RPAREN);
    emit_byte(parser, VM_OP_AALLOC);
}

static void parse_array_decl(parser_t *parser) {
    uint32_t count = 0;
    uint32_t start_pos = parser->cg->len;
    obj_arr_t *obj_arr = obj_arr_new(NULL, 0);

    // This is pretty gross, but we're going to eval all the expressions,
    // and then parse them ourselves, shoving each byte into the array.
    if (!check_token_tag(parser, TAG_RSQUIGGLY) &&
        !check_token_tag(parser, TAG_EOF)) {
        do {

            parse_expr_by_precedence(parser, PRECED_LOGICAL_OR);

            if (count > BYTEARRAY_MAX) {
                parser_error(parser, COMP_ERR_TOO_MANY_ELEMENTS,
                             "Number of elements in by array exceeds max %d", BYTEARRAY_MAX);
                return;
            }

            token_tag_match_and_consume(parser, TAG_EOL);

            count++;

            // TODO Check that each expr evaluated to a single byte.
        } while (token_tag_match_and_consume(parser, TAG_COMMA));
    }

    // Bytes are all present in bytecode.
    // Steal them and put them in a bytearray.
    uint32_t end_pos = parser->cg->len;

    // Avert your eyes.
    // Parse what we just compiled.
    uint32_t offset = start_pos;
    uint8_t code;
    while (offset < end_pos) {
        switch (code = parser->cg->bytecode[offset++]) {
            case VM_OP_BPUSH:
            case VM_OP_IPUSH:
                obj_arr_append(obj_arr, parser->cg->bytecode[offset++]);
                break;
            case VM_OP_IPUSH_0:
                obj_arr_append(obj_arr, 0);
                break;
            case VM_OP_IPUSH_1:
                obj_arr_append(obj_arr, 1);
                break;
            default:
                // This will not report the location accurately, since we're modifying the output.
                parser_error(parser, COMP_ERR_NON_BYTE_IN_BYTEARRAY,
                             "Invalid bytecode %s constructing byte array", op_names[code < VM_OP_MAX ? code : 0]);
                offset = end_pos;
                break;
        }
    }
    // Back up to the array
    parser->cg->len = start_pos;

    emit_const_obj_arr(parser, obj_arr);

    eat(parser, TAG_RSQUIGGLY);
}

static void parse_array_expr(parser_t *parser) {
    uint32_t arr_size = 0;

    if (token_tag_match_and_consume(parser, TAG_LPAREN)) {
        // array(12)
        parse_parens(parser);
        emit_byte(parser, VM_OP_AALLOC);
    } else if (token_tag_match_and_consume(parser, TAG_LSQUIGGLY)) {
        // array { 1, 2, 3 }
        parse_array_decl(parser);
    } else {
        parser_error(parser, COMP_ERR_UNEXPECTED_TOKEN,
                     "Unexpected token in array expression: %s", tag_names[parser->curr.tag]);
    }
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_array(parser_t *parser) {
    parse_array_expr(parser);
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
            parser_error(parser, COMP_ERR_UNHANDLED_LITERAL,
                         "Unexpected token while parsing literal: %s", tag_names[parser->prev.tag]);
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
            parser_error(parser, COMP_ERR_UNHANDLED_PREFIX_OP,
                         "No handler for prefix op: %s", op_names[op]);
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
            parser_error(parser, COMP_ERR_UNHANDLED_INFIX_OP,
                         "No handler for infix op: %s", op_names[op]);
            break;
    }
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_parens(parser_t *parser) {
    while (!check_token_tag(parser, TAG_RPAREN) &&
           !check_token_tag(parser, TAG_EOF)) {
        parse_decl(parser);
        token_tag_match_and_consume(parser, TAG_EOL);
    }
    eat(parser, TAG_RPAREN);
}

// Referenced via pointer in the precedence table.
__attribute__((unused)) void parse_subscript(parser_t *parser) {
    while (!check_token_tag(parser, TAG_RBRACKET) &&
           !check_token_tag(parser, TAG_EOF)) {
        parse_expr(parser);
        token_tag_match_and_consume(parser, TAG_EOL);
    }
    eat(parser, TAG_RBRACKET);
    emit_byte(parser, VM_OP_ALOAD);
}

static void parse_expr(parser_t *parser) {
    parse_expr_by_precedence(parser, PRECED_ASSIGN);
}

static void parse_print(parser_t *parser) {
    eat(parser, TAG_LPAREN);
    parse_parens(parser);
    emit_byte(parser, VM_OP_PRINT);
}

static void parse_if_stmt(parser_t *parser) {
    uint16_t from_if_addr;
    uint16_t from_else_addr;

    eat(parser, TAG_LPAREN);
    parse_parens(parser);

    eat(parser, TAG_THEN);

    from_if_addr = emit_jump(parser, VM_OP_JZ);

    parse_stmt(parser);

    if (token_tag_match_and_consume(parser, TAG_ELSE)) {
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
    while (!token_tag_match_and_consume(parser, TAG_RSQUIGGLY) &&
           !token_tag_match_and_consume(parser, TAG_EOF)) {
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
 *       | array_expr
 *       | expr
 *       | block
 *
 * block -> "{" decl* "}"
 */
static void parse_stmt(parser_t *parser) {
    if (token_tag_match_and_consume(parser, TAG_PRINT)) {
        parse_print(parser);
    } else if (token_tag_match_and_consume(parser, TAG_IF)) {
        parse_if_stmt(parser);
    } else if (token_tag_match_and_consume(parser, TAG_ARRAY)) {
        parse_array_expr(parser);
    } else {
        parse_expr(parser);
    }
}

/*
 * Parse declaration
 *
 * decl -> stmt
 *       | var_decl
 */
static void parse_decl(parser_t *parser) {
    parse_stmt(parser);
    token_tag_match_and_consume(parser, TAG_EOL);
}

comp_err_t codegen(const char *input, cg_t *cg) {
    printf("Compiling input: %s\n", input);
    lexer_t lexer;
    lexer_init(&lexer, input);

    parser_t parser;
    parser.lexer = &lexer;
    parser.cg = cg;
    parser.err = COMP_ERR_NO_ERROR;

    advance(&parser);
    while (!token_tag_match_and_consume(&parser, TAG_EOF)) {
        parse_decl(&parser);
    }
    eat(&parser, TAG_EOF);
    emit_byte(&parser, VM_OP_RET);

    print_dis(parser.cg);

    return parser.err;
}

comp_err_t compile(const cg_t *cg, uint32_t max_size, uint8_t buf[], uint32_t *size) {
    if (max_size < MIN_BYTECODE_ALLOC) {
        return COMP_ERR_INSUFFICIENT_SPACE_FOR_BYTECODE;
    }

    uint32_t offset;

    // Header
    memcpy(buf, magic, 4);
    offset = 4;
    buf[offset++] = major;
    buf[offset++] = minor;

    // Const pool
    if (cg->consts->buckets->nelems > UINT8_MAX) {
        return COMP_ERR_TOO_MANY_CONSTANTS;
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
                printf("Can't compile const type %d\n", v->type);
                exit(1);
        }
    }

    *size = offset;
    /*
    for (uint8_t i = 0; i < offset; i++) {
        printf("buf[%d]\t%x\n", i, buf[i]);
    }
     */

    return COMP_ERR_NO_ERROR;
}
