#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.h"
#include "ast.h"

ast_expr_t *parse_program(char* input);

#endif

