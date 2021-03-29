#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "../inc/token.h"
#include "../inc/lexer.h"

#define MAX_WORD 256
char word_buf[MAX_WORD];
char next_word_buf[MAX_WORD];

static void unreadch(lexer_t *lexer) {
  lexer->pos--;
  lexer->buf[lexer->pos] = lexer->nextch;
  lexer->nextch = lexer->pos > 0 ? lexer->buf[lexer->pos-1] : '\0';
}

static void readch(lexer_t *lexer) {
  lexer->nextch = lexer->buf[lexer->pos++];
}

static void consume_ws(lexer_t *lexer) {
  for (;;) {
    readch(lexer);
    switch (lexer->nextch) {
    case ' ':
    case '\t':
    case '\r':
      // Do not eat EOL.
      /* continue */
      break;

    default:
      return;
    }
  }
}

static token_t *lexer_error(lexer_t *lexer) {
  lexer->next_token.tag = TAG_EOF;
  lexer->err = LEX_UNEXPECTED_TOKEN;
  // The error was at the previous pos.
  lexer->err_pos = lexer->pos > 0 ? lexer->pos - 1 : 0;
  return &lexer->next_token;
}

token_t *lex_eof(lexer_t *lexer) {
  lexer->next_token.tag = TAG_EOF;
  return &lexer->next_token;
}

token_t *lex_eol(lexer_t *lexer) {
  lexer->next_token.tag = TAG_EOL;
  return &lexer->next_token;
}

/**
 * Consume all text up to the end of the line; return EOL.
 */
static token_t *lex_comment(lexer_t *lexer) {
  while (lexer->nextch != 0 && lexer->nextch != '\n') {
    readch(lexer);
  }

  return lex_eol(lexer);
}

/**
 * Lex an int or float number.
 *
 * Float numbers do not need an initial decimal digit.
 * I.e., 0.123 and .123 are equally ok.
 */
static token_t *lex_num(lexer_t *lexer) {
  int i = 0;
  int f = 0;
  int sign = 1;
  float frac = 1.0;

  // Integer part from digits up to any non-digit character.
  if (lexer->nextch >= '0' && lexer->nextch <= '9') {
    do {
      i = i * 10 + (lexer->nextch - '0');
      readch(lexer);
    } while (lexer->nextch >= '0' && lexer->nextch <= '9');
  }

  if (lexer->nextch == '.') {
    readch(lexer);

    // Make sure this isn't actually a range!
    if (lexer->nextch == '.') {
      unreadch(lexer);
      unreadch(lexer);
      goto done;
    }

    do {
      f = f * 10 + (lexer->nextch - '0');
      // What a mess.
      frac *= 0.1F;
      readch(lexer);
    } while (lexer->nextch >'0' && lexer->nextch <= '9');
  }

  unreadch(lexer);

done:
  if (!f) {
    lexer->next_token.tag = TAG_INT;
    lexer->next_token.intval = sign * i;
  } else {
    lexer->next_token.tag = TAG_FLOAT;
    lexer->next_token.floatval = sign * ((float) i) + ((float) f * frac);
  }

  return &lexer->next_token;
}

static void lex_word_raw(lexer_t *lexer) {
  memset(next_word_buf, 0, MAX_WORD);
  int i = 0;
  do {
    next_word_buf[i++] = lexer->nextch;
    readch(lexer);
  } while (isalnum(lexer->nextch));
  unreadch(lexer);
}

static token_t *lex_word(lexer_t *lexer) {
  lex_word_raw(lexer);

  // Is it a type declaration following the "of" keyword?
  if (lexer->token.tag == TAG_OF) {
    lexer->next_token.tag = TAG_TYPE_NAME;
    lexer->next_token.string = next_word_buf;
    return &lexer->next_token;
  }

  // Is it a reserved word?
  for (int j = 0; j < sizeof(reserved) / sizeof(reserved[0]); j++) {
    if (!strcmp(reserved[j].string, next_word_buf)) { 
      lexer->next_token = reserved[j];
      return &lexer->next_token;
    }
  }

  // Otherwise just an identifier created by the user.
  lexer->next_token.tag = TAG_IDENT;
  lexer->next_token.string = next_word_buf;
  return &lexer->next_token;
}

static token_t *lex_paren(lexer_t *lexer, tag_t which) {
  lexer->next_token.tag = which;
  return &lexer->next_token;
}

static token_t *lex_op(lexer_t *lexer, tag_t op) {
  lexer->next_token.tag = op;
  return &lexer->next_token;
}

static token_t *lex_char(lexer_t *lexer) {
  readch(lexer);
  char c = lexer->nextch;
  if (c == '\'') return lexer_error(lexer);
  readch(lexer);
  if (lexer->nextch != '\'') return lexer_error(lexer);

  lexer->next_token.tag = TAG_CHAR;
  lexer->next_token.ch = c;
  return &lexer->next_token;
}

static token_t *lex_field_or_method(lexer_t *lexer) {
  lex_word_raw(lexer);

  consume_ws(lexer);

  if (lexer->nextch == '(') {
    unreadch(lexer);
    lexer->next_token.tag = TAG_METHOD_NAME;
    lexer->next_token.string = next_word_buf;
    return &lexer->next_token;
  }

  unreadch(lexer);
  lexer->next_token.tag = TAG_FIELD_NAME;
  lexer->next_token.string = next_word_buf;
  return &lexer->next_token;
}

static token_t *lex_string(lexer_t *lexer) {
  // Eat initial quote.
  readch(lexer);
  memset(next_word_buf, 0, MAX_WORD);
  int i = 0;
  while (lexer->nextch != '"') {
    next_word_buf[i++] = lexer->nextch;
    readch(lexer);
  } 
  // Ate final quote.

  lexer->next_token.tag = TAG_STRING;
  // The string value is moved in the advance() function.
  return &lexer->next_token;
}

bool is_whitespace(char c) {
  switch(c) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      return true;
  }
  return false;
}

token_t *get_token(lexer_t *lexer) {
  consume_ws(lexer);
  char ch = lexer->nextch;

  if (ch == 0) return lex_eof(lexer);

  if (ch == '\n') return lex_eol(lexer);

  if (ch >= '0' && ch <= '9') return lex_num(lexer);

  if ((ch >= 'a' && ch <= 'z') ||
      (ch >= 'A' && ch <= 'Z')) return lex_word(lexer);
       
  switch(ch) {
    case ';': return lex_comment(lexer);
    case '{': return lex_paren(lexer, TAG_BEGIN);
    case '}': return lex_paren(lexer, TAG_END);
    case '(': return lex_paren(lexer, TAG_LPAREN);
    case ')': return lex_paren(lexer, TAG_RPAREN);
    case ',': return lex_paren(lexer, TAG_COMMA);
    case '+': return lex_op(lexer, TAG_PLUS);
    // The parser can treat this as the sign op or a binop as context dictates.
    case '-': return lex_op(lexer, TAG_MINUS);
    case '*': return lex_op(lexer, TAG_TIMES);
    case '/': return lex_op(lexer, TAG_DIVIDE);
    case '.': {
      readch(lexer);
      if (lexer->nextch == '.') {
        return lex_op(lexer, TAG_RANGE);
      }

      // Start of a float number?
      if (lexer->nextch >= '0' && lexer->nextch <= '9') {
        unreadch(lexer);
        return lex_num(lexer);
      }

      // Otherwise start of a field or method access.
      // No unreadch of the '.'.
      return lex_field_or_method(lexer);
    }
    case '<': {
      readch(lexer);
      if (lexer->nextch == '=') {
        return lex_op(lexer, TAG_LE);
      }
      unreadch(lexer);
      return lex_op(lexer, TAG_LT);
    }
    case '>': {
      readch(lexer);
      if (lexer->nextch == '=') {
        return lex_op(lexer, TAG_GE);
      }
      unreadch(lexer);
      return lex_op(lexer, TAG_GT);
    }
    case '=': {
      readch(lexer);
      if (lexer->nextch == '=') {
        return lex_op(lexer, TAG_EQ);
      }
      return lex_op(lexer, TAG_ASSIGN);
    }
    case '!': {
      readch(lexer);
      if (lexer->nextch == '=') {
        return lex_op(lexer, TAG_NE);
      }
      // TODO: NOT, probably like NEG
      return lexer_error(lexer);
    }
    case '\'': return lex_char(lexer);
    case '"': return lex_string(lexer);
  }

  return lexer_error(lexer);
}

void advance(lexer_t *lexer) {
  lexer->token = lexer->next_token;
  switch(lexer->token.tag) {
    case TAG_INT:
      lexer->token.intval = lexer->next_token.intval;
      break;
    case TAG_FLOAT:
      lexer->token.floatval = lexer->next_token.floatval;
      break;
    case TAG_CHAR:
      lexer->token.ch = lexer->next_token.ch;
      break;
    default:
      for (int i = 0; i < MAX_WORD; i++) {
        word_buf[i] = next_word_buf[i];
      }
      lexer->token.string = word_buf;
      break;
  }

  if (lexer->token.tag != TAG_EOF) {
    lexer->next_token = *get_token(lexer);
  }
}

bool eat(lexer_t *lexer, tag_t t) {
  if (lexer->token.tag != t) {
    lexer->err_pos = (int) lexer->pos;
    lexer->err = LEX_ERROR;
    return false;
  }

  advance(lexer);
  return true;
}

void lexer_init(lexer_t *lexer, const char input[], const uint32_t input_size) {
  lexer->pos = 0;
  lexer->err_pos = 0;
  lexer->depth = 1;
  lexer->err = NO_ERROR;

  memset(next_word_buf, 0, MAX_WORD);
  memset(word_buf, 0, MAX_WORD);

  memset(lexer->buf, 0, LEXER_BUF_SIZE);
  for (uint8_t i = 0; i < input_size; i++) {
    lexer->buf[i] = input[i];
  }

  lexer->token.tag = TAG_EOF;
  lexer->next_token = *get_token(lexer);
  advance(lexer);
}

