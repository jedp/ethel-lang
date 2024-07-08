#include <stdlib.h>
#include "../common/op.h"
#include "cg.h"
#include "comp.h"
#include "lex.h"

static void error(parser_t *parser, uint8_t which) {
    parser->err = which;
}

static void advance(parser_t *parser) {
    parser->prev = parser->curr;

    while (1) {
        parser->curr = next_token(parser->lexer);

        if (parser->curr.tag == TAG_ERROR || parser->curr.tag == TAG_EOF) {
            break;
        }

        printf("parse: %d\n", parser->curr.tag);
    }
}

static void eat(parser_t *parser, tag_t tag) {
    if (parser->curr.tag != tag) {
        error(parser, COMP_UNEXPECTED_TOKEN);
        return;
    }

    advance(parser);
}

static void comp_byte(parser_t *parser, uint8_t byte) {
    cg_byte(parser->cg, byte);
}

static void comp_bytes(parser_t *parser, uint8_t byte1, uint8_t byte2) {
    comp_byte(parser, byte1);
    comp_byte(parser, byte2);
}

static void expr(parser_t *parser) {
}

static void comp_const(parser_t *parser, int val) {
    switch (val) {
        case -1:
            comp_byte(parser, VM_OP_LOADI_1N);
            break;
        case 0:
            comp_byte(parser, VM_OP_LOADI_0);
            break;
        case 1:
            comp_byte(parser, VM_OP_LOADI_1);
            break;
        default: {
            map_elem_t v = {.type = MAP_ELEM_INT_TYPE, .elem.intval = 42};
            uint8_t k;
            cg_put_const(parser->cg, v, &k);
            comp_bytes(parser, VM_OP_LOADI,k);
            break;
        }
    }
}

static void comp_op(parser_t *parser, vm_op_t op) {

}

static void parse_int(parser_t *parser) {
    comp_const(parser, (int) strtol(parser->prev.start, NULL, 10));
}

static void parse_preced(parser_t *parser, uint8_t preced) {

}

static void parse_expr(parser_t *parser) {
    parse_preced(parser, PRECED_NONE);
}

error_t comp(const char *input) {
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

    return parser.err;
}
