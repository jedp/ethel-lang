#include <stdlib.h>
#include "../common/op.h"
#include "cg.h"
#include "comp.h"
#include "dis.h"
#include "lex.h"
#include "preced.h"

static void parse_expr(parser_t *parser);

static void error(parser_t *parser, uint8_t which) {
    printf("ERROR: %d\n", which);
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
    printf("eat %d\n", tag);
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
    printf("emit %d %d\n", byte1, byte2);
    emit_byte(parser, byte1);
    emit_byte(parser, byte2);
}

static map_err_t emit_const(parser_t *parser, int val) {
    map_err_t err = MAP_OK;

    if (val == -1) {
        emit_byte(parser, VM_OP_LOADI_1N);
    } else if (val == 0) {
        emit_byte(parser, VM_OP_LOADI_0);
    } else if (val == 1) {
        emit_byte(parser, VM_OP_LOADI_1);
    } else if (val >= -128 && val <= 127) {
        emit_bytes(parser, VM_OP_BPUSH, (uint8_t) val & 0xff);
    } else {
        map_elem_t v = {.type = MAP_ELEM_INT_TYPE, .elem.intval = val};
        uint8_t k;
        err = cg_put_const(parser->cg, v, &k);
        emit_bytes(parser, VM_OP_LOADI, k);
    }

    return err;
}

static void emit_op(parser_t *parser, vm_op_t op) {

}

void parse_expr_by_precedence(parser_t *parser, uint8_t min_preced) {
    advance(parser);
    tag_t tag = parser->prev.tag;
    printf("pase by preced: tag %d\n", tag);
    parse_func prefix_rule = preced_rules[tag].parse_prefix;

    if (prefix_rule == NULL) {
        printf("prefix rule is null for tag %d!\n", tag);
        error(parser, COMP_EXPECTED_EXPRESSION);
        return;
    }

    prefix_rule(parser);

    // Pratt precedence climbing.
    while (min_preced <= preced_rules[parser->curr.tag].precedence) {
        advance(parser);
        parse_func infix_rule = preced_rules[parser->prev.tag].parse_infix;
        printf("climb: tag %d. func is %p\n", parser->prev.tag, infix_rule);
        if (infix_rule != NULL) infix_rule(parser);
    }
}

void parse_int(parser_t *parser) {
    int const_int = (int) strtol(parser->prev.start, NULL, 10);
    (void) emit_const(parser, const_int);
}

void parse_unary_op(parser_t *parser) {
    tag_t op = parser->prev.tag;

    parse_expr_by_precedence(parser, PRECED_UNARY);

    if (op == TAG_MINUS) {
        emit_byte(parser, VM_OP_NEGATE);
    } else {
        error(parser, COMP_UNHANDLED_PREFIX_OP);
    }
}

void parse_binary_op(parser_t *parser) {
    tag_t op = parser->prev.tag;

    // Parse and push the remainder of the expression.
    parse_preced_rule_t op_rule = preced_rules[op];
    // TODO The +1 is only for left-associative operators.
    parse_expr_by_precedence(parser, (preced_t) op_rule.precedence + 1);

    // Push the operator last.
    switch (op) {
        case TAG_PLUS:
            printf("emit +\n");
            emit_byte(parser, VM_OP_ADD);
            break;
        case TAG_MINUS:
            printf("emit -\n");
            emit_byte(parser, VM_OP_SUB);
            break;
        case TAG_TIMES:
            printf("emit *\n");
            emit_byte(parser, VM_OP_MUL);
            break;
        case TAG_DIVIDE:
            printf("emit /\n");
            emit_byte(parser, VM_OP_DIV);
            break;
        default:
            error(parser, COMP_UNHANDLED_INFIX_OP);
            break;
    }
}

void parse_parens(parser_t *parser) {
    parse_expr(parser);
    printf("parsed parens expr\n");
//    eat(parser, TAG_RPAREN);
}

void parse_subscript(parser_t *parser) {

}

static void parse_expr(parser_t *parser) {
    parse_expr_by_precedence(parser, PRECED_NONE);
}

error_t comp(const char *input) {
    printf("Compiling input: %s\n", input);
    lexer_t lexer;
    lexer_init(&lexer, input);

    cg_t cg;
    cg_init(&cg);

    parser_t parser;
    parser.lexer = &lexer;
    parser.cg = &cg;
    parser.err = COMP_ERR_NO_ERROR;

    advance(&parser);
    parse_expr(&parser);
    eat(&parser, TAG_EOF);
    emit_byte(&parser, VM_OP_RET);

    print_dis(parser.cg);

    return parser.err;
}
