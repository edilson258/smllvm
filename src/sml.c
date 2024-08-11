#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

#include "ast.h"
#include "lexer.h"
#include "llvm_gen.h"
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
  LLVMModuleRef module = llvm_emit_module(ast);
  LLVMDumpModule(module);
  return 0;
}
