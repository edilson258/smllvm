#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

#include "ast.h"
#include "lexer.h"
#include "llvm_gen.h"
#include "parser.h"
#include "utils.h"

void print_usage();
char *read_file(char *path);

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "[Error] Missing source file\n");
    print_usage();
    return 1;
  }

  char *source_file = argv[1];

  Lexer lexer = Lexer_New(read_file(source_file));
  Parser parser = Parser_New(lexer);
  StmtBlock ast = Parse(&parser);
  AST_Inspect(ast);

  LLVMModuleRef module = llvm_emit_module(ast, source_file);
  LLVMPrintModuleToFile(module, change_file_ext(source_file, ".ll"), 0);

  LLVMDisposeModule(module);

  return 0;
}

void print_usage() {
  printf("\nUsage:\n");
  printf("\tsmlc source_file\n");
}

char *read_file(char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp) {
    fprintf(stderr, "[Error] Couldn't open file %s: %s\n", path,
            strerror(errno));
  }

  fseek(fp, 0, SEEK_END);
  size_t file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *contents = malloc(sizeof(char) * (file_size + 1));
  fread(contents, sizeof(char), file_size, fp);
  contents[file_size] = '\00';

  return contents;
}
