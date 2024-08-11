#ifndef LLVM_CODE_GEN
#define LLVM_CODE_GEN

#include <llvm-c/Types.h>

#include "ast.h"

LLVMModuleRef llvm_emit_module(AST);

#endif
