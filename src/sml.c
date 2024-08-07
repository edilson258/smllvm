#include "lexer.h"
#include "token.h"

static char *program = "function main() -> int {"
                       "    print(\"Hello, world!\");"
                       "    return 69;"
                       "}";

int main(int argc, char *argv[]) {
  Lexer lexer = Lexer_New(program);
  Token token = Lexer_NextToken(&lexer);
  while (token.type != TOKEN_EOF) {
    Token_Inspect(&token);
    token = Lexer_NextToken(&lexer);
  }
  return 0;
}
