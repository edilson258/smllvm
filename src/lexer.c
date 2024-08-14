#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "token.h"

typedef bool (*LexerPredicate)(char);

static inline void read_char(Lexer *);
static inline void skip_whitespace(Lexer *);
static inline bool is_next_char(Lexer *, char);
static inline char *read_while(Lexer *, LexerPredicate);

// predicates
static inline bool endof_string_predicate(char x) { return x != '"'; }
static inline bool ident_predicate(char x) { return isalnum(x) || x == '_'; }
static inline bool number_predicate(char x) { return isdigit(x); }

Token Lexer_NextToken(Lexer *l) {
  skip_whitespace(l);

  Token token;
  token.position.line = l->line;
  token.position.colm = l->colm;

  if (l->curr_char == 0) {
    token.type = TOKEN_EOF;
    return token;
  }

  switch (l->curr_char) {
  case '(':
    read_char(l);
    token.type = TOKEN_LPAREN;
    return token;
  case ')':
    read_char(l);
    token.type = TOKEN_RPAREN;
    return token;
  case '{':
    read_char(l);
    token.type = TOKEN_LBRACE;
    return token;
  case '}':
    read_char(l);
    token.type = TOKEN_RBRACE;
    return token;
  case ';':
    read_char(l);
    token.type = TOKEN_SEMICOLON;
    return token;
  case '+':
    read_char(l);
    token.type = TOKEN_PLUS;
    return token;
  case '=':
    read_char(l);
    token.type = TOKEN_EQUAL;
    return token;
  case '-':
    if (is_next_char(l, '>')) {
      read_char(l); // eat '-'
      read_char(l); // eat '>'
      token.type = TOKEN_ARROW;
      return token;
    }
    break;
  case '"':
    read_char(l);
    token.type = TOKEN_STRING;
    token.value.string = read_while(l, endof_string_predicate);
    read_char(l);
    return token;
  }

  if (isalpha(l->curr_char) || l->curr_char == '_') {
    char *label = read_while(l, ident_predicate);
    bool can_free_label = true;

    if (strcmp(label, "function") == 0) {
      token.type = TOKEN_FN_DECL;
    } else if (strcmp(label, "int") == 0) {
      token.type = TOKEN_TYPE_INT;
    } else if (strcmp(label, "return") == 0) {
      token.type = TOKEN_RETURN;
    } else if (strcmp(label, "let") == 0) {
      token.type = TOKEN_LET;
    } else {
      token.type = TOKEN_IDENT;
      token.value.string = label;
      can_free_label = false;
    }

    if (can_free_label) {
      free(label);
    }

    return token;
  }

  if (isdigit(l->curr_char)) {
    char *label = read_while(l, number_predicate);
    token.type = TOKEN_NUMBER;
    token.value.number = strtoll(label, NULL, 10);
    free(label);
    return token;
  }

  token.type = TOKEN_ILLEGAL;
  token.value.char_ = l->curr_char;
  read_char(l);
  return token;
}

Lexer Lexer_New(char *buffer) {
  Lexer l;
  l.buffer = buffer;
  l.buffer_len = strlen(buffer);
  l.line = 1;
  l.colm = 0;
  l.pos = 0;
  l.read_pos = 0;
  l.curr_char = 0;
  read_char(&l);
  return l;
}

void skip_whitespace(Lexer *l) {
  while (l->curr_char != 0 && isspace(l->curr_char)) {
    read_char(l);
  }
}

void read_char(Lexer *l) {
  l->pos = l->read_pos;

  if (l->read_pos >= l->buffer_len) {
    l->curr_char = 0;
    return;
  }

  l->curr_char = l->buffer[l->read_pos];
  l->read_pos += 1;

  if (l->curr_char == '\n') {
    l->line += 1;
    l->colm = 0;
  } else {
    l->colm += 1;
  }
}

bool is_next_char(Lexer *l, char x) {
  if (l->read_pos >= l->buffer_len) {
    return false;
  }
  return l->buffer[l->read_pos] == x;
}

char *read_while(Lexer *l, LexerPredicate pred) {
  size_t start = l->pos;
  while (l->curr_char != 0 && pred(l->curr_char)) {
    read_char(l);
  }
  size_t len = l->pos - start;
  char *substr = (char *)malloc(sizeof(char) * (len + 1));
  strncpy(substr, l->buffer + start, len);
  substr[len] = 0;
  return substr;
}
