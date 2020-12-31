#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

#define MAX_WORD 16
char word_buf[MAX_WORD];

void lexer_init(lexer_t *lexer, const char input[], const uint8_t input_size) {
  lexer->pos = 0;
  lexer->err = -1;
  lexer->peek = ' ';

  memset(lexer->buf, 0, LEXER_BUF_SIZE);
  for (uint8_t i = 0; i < input_size; i++) {
    lexer->buf[i] = input[i];
  }
}

static void readch(lexer_t *lexer) {
  lexer->peek = lexer->buf[lexer->pos++];
}

static bool nextch(lexer_t *lexer, char c) {
  readch(lexer);
  if (lexer->peek != c) {
    return false;
  }

  lexer->peek = ' ';
  return true;
}

static void consume_ws(lexer_t *lexer) {
  for (;;) {
    readch(lexer);
    switch (lexer->peek) {
    case ' ':
    case '\t':
      /* continue */
      break;

    default:
      return;
    }
  }
}

static token_t *lexer_error(lexer_t *lexer) {
  lexer->token.tag = _EOF;
  lexer->token.value = -1;
  // The error was at the previous pos.
  lexer->err = lexer->pos > 0 ? lexer->pos - 1 : 0;
  return &lexer->token;
}

static token_t *lex_eof(lexer_t *lexer) {
  lexer->token.tag = _EOF;
  lexer->token.value = -1;
  return &lexer->token;
}

static token_t *lex_num(lexer_t *lexer) {
  int v = 0;
  do {
    v = v * 10 + (lexer->peek - '0');
    readch(lexer);
  } while (lexer->peek >= '0' && lexer->peek <= '9');

  lexer->token.tag = INT;
  lexer->token.value = v;
  return &lexer->token;
}

static token_t *lex_word(lexer_t *lexer) {
  memset(word_buf, 0, MAX_WORD);
  int i = 0;
  do {
    word_buf[i++] = lexer->peek;
    readch(lexer);
  } while (isalnum(lexer->peek));

  // A hash map might be faster.
  for (int j = 0; j < sizeof(reserved) / sizeof(reserved[0]); j++) {
    if (!strcmp(reserved[j].string, word_buf)) { 
      lexer->token = reserved[j];
      return &lexer->token;
    }
  }

  lexer->token.tag = VARNAME;
  lexer->token.string = word_buf;
  return &lexer->token;
}

static token_t *lex_binop(lexer_t *lexer, tag_t op) {
  lexer->token.tag = op;
  lexer->token.value = -1;
  return &lexer->token;
}

token_t *lexer_next(lexer_t *lexer) {
  consume_ws(lexer);

  if (lexer->peek == 0) return lex_eof(lexer);

  if (lexer->peek >= '0' && lexer->peek <= '9') return lex_num(lexer);

  if ((lexer->peek >= 'a' && lexer->peek <= 'z') ||
      (lexer->peek >= 'A' && lexer->peek <= 'Z')) return lex_word(lexer);
       
  switch(lexer->peek) {
    case '+': return lex_binop(lexer, ADD);
    case '-': return lex_binop(lexer, SUB);
    case '*': return lex_binop(lexer, MUL);
    case '/': return lex_binop(lexer, DIV);
    case '<': return nextch(lexer, '=') ? lex_binop(lexer, LE) : lex_binop(lexer, LT);
    case '>': return nextch(lexer, '=') ? lex_binop(lexer, GE) : lex_binop(lexer, GT);
  }

  return lexer_error(lexer);
}

