#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

#include "ast.h"
#include "llvm_gen.h"
#include "stdlib.h"
#include "utils.h"

StdLib *stdlib;
LLVMModuleRef llvm_module;
LLVMBuilderRef llvm_builder;
LLVMContextRef llvm_context;

LLVMTypeRef sml_to_llvm_type(Type);
void llvm_emit_stmt_block(StmtBlock);
void llvm_emit_stmt_function(StmtFnDecl);
LLVMValueRef llvm_emit_stmt_expr(StmtExpr);
LLVMValueRef llvm_emit_expr_call(ExprCall);
LLVMValueRef llvm_emit_expr_binop(ExprBinOp);
LLVMValueRef llvm_emit_expr_literal(ExprLiteral);

LLVMModuleRef llvm_emit_module(AST ast, char *source_file) {
  init_std_lib(&stdlib);
  llvm_module = LLVMModuleCreateWithName("hello");
  llvm_context = LLVMContextCreate();
  llvm_builder = LLVMCreateBuilderInContext(llvm_context);
  LLVMSetSourceFileName(llvm_module, source_file, strlen(source_file));

  llvm_emit_stmt_block(ast);

  LLVMDisposeBuilder(llvm_builder);
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
  case EXPR_BINOP:
    return llvm_emit_expr_binop(expr.value.binop);
  default:
    puts("Not supported expression\n");
    exit(1);
  }
}

LLVMValueRef llvm_emit_expr_call(ExprCall call) {
  BuiltinFn *fn = find_builtin_fn(stdlib, call.callee);

  assert(fn && "Calling non-defined function\n");
  assert(fn->prototype.param_count == call.args.argc &&
         "Args count miss match\n");

  LLVMTypeRef return_type = sml_to_llvm_type(fn->prototype.return_type);

  LLVMTypeRef params[fn->prototype.param_count];
  for (size_t i = 0; i < fn->prototype.param_count; ++i) {
    params[i] = sml_to_llvm_type(fn->prototype.param_types[i]);
  }

  LLVMTypeRef fn_type =
      LLVMFunctionType(return_type, params, fn->prototype.param_count, 1);
  LLVMValueRef val = LLVMAddFunction(llvm_module, fn->prototype.name, fn_type);

  LLVMValueRef args[call.args.argc];
  for (size_t i = 0; i < call.args.argc; ++i) {
    args[i] = llvm_emit_stmt_expr(call.args.argv[i]);
  }

  val = LLVMBuildCall2(llvm_builder, fn_type, val, args, call.args.argc, "");
  return val;
}

LLVMValueRef llvm_emit_expr_binop(ExprBinOp binop) {
  LLVMValueRef llvm_lhs = llvm_emit_stmt_expr(*binop.lhs);
  LLVMValueRef llvm_rhs = llvm_emit_stmt_expr(*binop.rhs);

  LLVMValueRef llvm_binop;
  switch (binop.op) {
  case BINOP_PLUS:
    llvm_binop = LLVMBuildAdd(llvm_builder, llvm_lhs, llvm_rhs, "");
    break;
  }

  return llvm_binop;
}

LLVMValueRef llvm_emit_expr_literal(ExprLiteral literal) {
  switch (literal.type) {
  case EXPR_LITERAL_NUM:
    return LLVMConstInt(LLVMInt32Type(), literal.value.number, 0);
  case EXPR_LITERAL_STR: {
    char *unescaped_str = unescape_str(literal.value.string);
    return LLVMBuildGlobalStringPtr(llvm_builder, unescaped_str, "str");
  }
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
