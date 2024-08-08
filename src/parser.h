#ifndef SML_PARSER
#define SML_PARSER

#include "ast.h"
#include "lexer.h"
#include "token.h"

typedef struct {
  Lexer lexer;
  Token curr_token;
  Token next_token;
} Parser;

Parser Parser_New(Lexer lexer);
AST Parse(Parser *);

#endif
