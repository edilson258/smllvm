#ifndef SML_LEXER
#define SML_LEXER

#include "token.h"
#include <stddef.h>
#include <stdio.h>

typedef struct {
  char *buffer;
  size_t buffer_len;
  char curr_char;

  size_t line;
  size_t colm;

  size_t pos;
  size_t read_pos;
} Lexer;

Lexer Lexer_New(char *buffer);
Token Lexer_NextToken(Lexer *);

#endif
