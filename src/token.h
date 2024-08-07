#ifndef SML_TOKEN
#define SML_TOKEN

#include <stddef.h>
#include <stdio.h>

typedef enum {
  TOKEN_EOF = 1,
  TOKEN_ILLEGAL,

  TOKEN_IDENT,

  // values
  TOKEN_NUMBER,
  TOKEN_STRING,

  // symbols
  TOKEN_ARROW,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_SEMICOLON,

  // keywords
  TOKEN_RETURN,
  TOKEN_FN_DECL,
  TOKEN_TYPE_INT,
} TokenType;

typedef struct {
  TokenType type;
  union {
    char char_;
    char *string;
    long long number;
  } value;
  struct {
    size_t line;
    size_t colm;
  } position;
} Token;

void Token_Inspect(Token *token);

#endif
