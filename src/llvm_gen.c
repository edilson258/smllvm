#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

#include "llvm_gen.h"

LLVMModuleRef llvm_module;
LLVMBuilderRef llvm_builder;
LLVMContextRef llvm_context;

typedef struct {
  char *name;
  char *alias; // eg. 'print' is an alias of 'printf'
  size_t param_count;
  Type *param_types;
  Type return_type;
} NativeFn;

size_t native_fn_count = 1;
NativeFn *native_fns;

void setup_native_fns() {
  NativeFn print;
  print.alias = "print";
  print.name = "printf";
  print.param_count = 1;
  print.param_types = malloc(sizeof(Type) * 1);
  print.param_types[0] = TYPE_STR;
  print.return_type = TYPE_INT;

  native_fns = malloc(sizeof(NativeFn) * native_fn_count);
  native_fns[0] = print;
}

NativeFn *find_native_fn(char *alias) {
  for (size_t i = 0; i < native_fn_count; ++i) {
    if (strcmp(native_fns[i].alias, alias) == 0) {
      return &native_fns[i];
    }
  }
  return 0;
}

LLVMTypeRef sml_to_llvm_type(Type);
LLVMModuleRef llvm_emit_module(AST);
void llvm_emit_stmt_block(StmtBlock);
void llvm_emit_stmt_function(StmtFnDecl);
LLVMValueRef llvm_emit_stmt_expr(StmtExpr);
LLVMValueRef llvm_emit_expr_call(ExprCall);
LLVMValueRef llvm_emit_expr_literal(ExprLiteral);

LLVMModuleRef llvm_emit_module(AST ast) {
  llvm_module = LLVMModuleCreateWithName("hello");
  llvm_context = LLVMContextCreate();
  llvm_builder = LLVMCreateBuilderInContext(llvm_context);
  setup_native_fns();

  llvm_emit_stmt_block(ast);

  return llvm_module;
}

void llvm_emit_stmt_block(StmtBlock block) {
  for (size_t i = 0; i < block.stmt_count; ++i) {
    Stmt stmt = block.stmts[i];
    switch (stmt.type) {
    case STMT_FN_DECL:
      llvm_emit_stmt_function(stmt.value.fn_decl);
      break;
    default:
      puts("Only top level stmt is allowed at global scope\n");
      exit(1);
    }
  }
}

void llvm_emit_stmt_function(StmtFnDecl decl) {
  LLVMTypeRef fn_ret_type = sml_to_llvm_type(decl.return_type);
  LLVMTypeRef fn_prototype = LLVMFunctionType(fn_ret_type, NULL, 0, 0);
  LLVMValueRef fn = LLVMAddFunction(llvm_module, decl.name, fn_prototype);
  LLVMBasicBlockRef fn_body = LLVMAppendBasicBlock(fn, "");
  LLVMPositionBuilderAtEnd(llvm_builder, fn_body);

  for (size_t i = 0; i < decl.body.stmt_count; ++i) {
    Stmt stmt = decl.body.stmts[i];
    switch (stmt.type) {
    case STMT_RETURN: {
      LLVMValueRef val = llvm_emit_stmt_expr(stmt.value.return_.operand);
      LLVMBuildRet(llvm_builder, val);
      break;
    }
    case STMT_EXPR: {
      LLVMValueRef _ = llvm_emit_stmt_expr(stmt.value.expr);
      break;
    }
    default:
      puts("Top level stmt not allowed inside function\n");
      exit(1);
    }
  }
}

LLVMValueRef llvm_emit_stmt_expr(StmtExpr expr) {
  switch (expr.type) {
  case EXPR_LITERAL:
    return llvm_emit_expr_literal(expr.value.literal);
  case EXPR_CALL:
    return llvm_emit_expr_call(expr.value.call);
  default:
    puts("Not supported expression\n");
    exit(1);
  }
}

LLVMValueRef llvm_emit_expr_call(ExprCall call) {
  NativeFn *fn = find_native_fn(call.callee);

  assert(fn && "Calling non-defined function\n");
  assert(fn->param_count == call.args.argc && "Args count miss match\n");

  LLVMValueRef val;

  LLVMTypeRef return_type = sml_to_llvm_type(fn->return_type);
  LLVMTypeRef params[fn->param_count];

  for (size_t i = 0; i < fn->param_count; ++i) {
    params[i] = sml_to_llvm_type(fn->param_types[i]);
  }

  LLVMTypeRef fn_type =
      LLVMFunctionType(return_type, params, fn->param_count, 1);
  val = LLVMAddFunction(llvm_module, fn->name, fn_type);

  LLVMValueRef args[fn->param_count];

  for (size_t i = 0; i < fn->param_count; ++i) {
    args[i] = llvm_emit_stmt_expr(call.args.argv[i]);
  }

  val = LLVMBuildCall2(llvm_builder, fn_type, val, args, fn->param_count, "");

  return val;
}

LLVMValueRef llvm_emit_expr_literal(ExprLiteral literal) {
  LLVMValueRef val;
  switch (literal.type) {
  case EXPR_LITERAL_NUM:
    val = LLVMConstInt(LLVMInt32Type(), literal.value.number, 0);
    return val;
  case EXPR_LITERAL_STR:
    val = LLVMBuildGlobalString(llvm_builder, literal.value.string, "str");
    return val;
  }
}

LLVMTypeRef sml_to_llvm_type(Type type) {
  switch (type) {
  case TYPE_INT:
    return LLVMInt32Type();
  case TYPE_STR:
    return LLVMPointerType(LLVMInt8Type(), 0);
  }
}
