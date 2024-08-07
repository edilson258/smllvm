#include "token.h"

void Token_Inspect(Token *token) {
  switch (token->type) {
  case TOKEN_EOF:
    printf("EOF ");
    break;
  case TOKEN_ARROW:
    printf("SYMBOL: -> ");
    break;
  case TOKEN_FN_DECL:
    printf("KEYWORD: function ");
    break;
  case TOKEN_IDENT:
    printf("IDENT: %s ", token->value.string);
    break;
  case TOKEN_STRING:
    printf("STRING: \"%s\" ", token->value.string);
    break;
  case TOKEN_NUMBER:
    printf("NUMBER: %lld ", token->value.number);
    break;
  case TOKEN_ILLEGAL:
    printf("ILLEGAL: \"%c\" ", token->value.char_);
    break;
  case TOKEN_LPAREN:
    printf("SYMBOL: ( ");
    break;
  case TOKEN_RPAREN:
    printf("SYMBOL: ) ");
    break;
  case TOKEN_LBRACE:
    printf("SYMBOL: { ");
    break;
  case TOKEN_RBRACE:
    printf("SYMBOL: } ");
    break;
  case TOKEN_SEMICOLON:
    printf("SYMBOL: ; ");
    break;
  case TOKEN_RETURN:
    printf("KEYWORD: return ");
    break;
  case TOKEN_TYPE_INT:
    printf("KEYWORD: int ");
    break;
  }

  printf("%zu:%zu\n", token->position.line, token->position.colm);
}
