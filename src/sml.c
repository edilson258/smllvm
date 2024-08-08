#include "ast.h"
#include "lexer.h"
#include "parser.h"

static char *program = "function main() -> int {"
                       "    print(\"Hello, world!\");"
                       "    return 69;"
                       "}";

int main(int argc, char *argv[]) {
  Lexer lexer = Lexer_New(program);
  Parser parser = Parser_New(lexer);
  StmtBlock ast = Parse(&parser);
  AST_Inspect(ast);
  return 0;
}
