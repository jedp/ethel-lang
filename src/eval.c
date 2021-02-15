#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../inc/eval.h"
#include "../inc/parser.h"
#include "../inc/ast.h"

int eval_int(ast_expr_t *expr) {
    assert(expr->type == AST_INT);
    return expr->intval;
}

float eval_float(ast_expr_t *expr) {
    assert(expr->type == AST_FLOAT);
    return expr->floatval;
}

eval_result_t *eval_expr(ast_expr_t *expr) {
    eval_result_t *result = malloc(sizeof(eval_result_t));
    result->err = AST_NO_ERROR;

    switch(expr->type) {
        case AST_ADD: {
            eval_result_t *r1 = eval_expr(expr->e1);
            eval_result_t *r2 = eval_expr(expr->e2);
            result->type = AST_INT;
            result->intval = r1->intval + r2->intval;
            break;
        }
        case AST_SUB: {
            eval_result_t *r1 = eval_expr(expr->e1);
            eval_result_t *r2 = eval_expr(expr->e2);
            result->type = AST_INT;
            result->intval = r1->intval - r2->intval;
            break;
        }
        case AST_MUL: {
            eval_result_t *r1 = eval_expr(expr->e1);
            eval_result_t *r2 = eval_expr(expr->e2);
            result->type = AST_INT;
            result->intval = r1->intval * r2->intval;
            break;
        }
        case AST_DIV: {
            eval_result_t *r1 = eval_expr(expr->e1);
            eval_result_t *r2 = eval_expr(expr->e2);
            result->type = AST_INT;
            result->intval = (int) (r1->intval / r2->intval);
            break;
        }
        case AST_INT:
            result->type = AST_INT;
            result->intval = eval_int(expr);
            break;
        case AST_FLOAT:
            result->type = AST_FLOAT;
            result->floatval = eval_float(expr);
            break;
        case AST_IDENT:
            break;
        default:
            result->err = AST_UNSUPPORTED_OPERATION;
            break;
    }

    return result;
}

eval_result_t *eval(char *input) {
    ast_expr_t *ast = parse_program(input);

#ifdef DEBUG
    pretty_print(ast);
#endif

    return eval_expr(ast);
}

