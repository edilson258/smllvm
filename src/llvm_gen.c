#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

#include "ast.h"
#include "llvm_gen.h"
#include "stdlib.h"
#include "type.h"
#include "utils.h"

StdLib *stdlib;
LLVMModuleRef llvm_module;
LLVMBuilderRef llvm_builder;
LLVMContextRef llvm_context;

LLVMTypeRef sml_to_llvm_type(Type);
void llvm_emit_stmt_block(StmtBlock);
void llvm_emit_stmt_function(StmtFnDecl);
void llvm_emit_stmt_vardecl(StmtVarDecl);
LLVMValueRef llvm_emit_stmt_expr(StmtExpr);
LLVMValueRef llvm_emit_expr_call(ExprCall);
LLVMValueRef llvm_emit_expr_binop(ExprBinOp);
LLVMValueRef llvm_emit_expr_literal(ExprLiteral);
LLVMValueRef llvm_emit_expr_ident(ExprIdent);

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
    case STMT_VAR_DECL:
      llvm_emit_stmt_vardecl(stmt.value.var_decl);
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

void llvm_emit_stmt_vardecl(StmtVarDecl var_decl) {
  LLVMValueRef llvm_init_val = llvm_emit_stmt_expr(*var_decl.init);

  if (LLVMIsConstantString(llvm_init_val)) {
    size_t str_len = 0;
    const char *str = LLVMGetAsString(llvm_init_val, &str_len);
    LLVMTypeRef llvm_str_type = LLVMArrayType2(LLVMInt8Type(), str_len);
    LLVMValueRef llvm_str =
        LLVMAddGlobal(llvm_module, llvm_str_type, var_decl.name);
    LLVMSetInitializer(llvm_str, llvm_init_val);
  }

  if (LLVMIsAConstantInt(llvm_init_val)) {
    LLVMValueRef llvm_int =
        LLVMAddGlobal(llvm_module, LLVMInt32Type(), var_decl.name);
    LLVMSetInitializer(llvm_int, llvm_init_val);
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
  case EXPR_IDENT:
    return llvm_emit_expr_ident(expr.value.ident);
    break;
  }
}

LLVMValueRef llvm_emit_expr_call(ExprCall call_expr) {
  BuiltinFn *called_fn = find_builtin_fn(stdlib, call_expr.name);

  assert(called_fn && "Calling non-defined function\n");
  assert(called_fn->prototype.param_count == call_expr.args.argc &&
         "Args count miss match\n");

  LLVMTypeRef llvm_ret_type =
      sml_to_llvm_type(called_fn->prototype.return_type);

  LLVMTypeRef llvm_params[called_fn->prototype.param_count];
  for (size_t i = 0; i < called_fn->prototype.param_count; ++i) {
    llvm_params[i] = sml_to_llvm_type(called_fn->prototype.param_types[i]);
  }

  LLVMTypeRef llvm_fn_type = LLVMFunctionType(
      llvm_ret_type, llvm_params, called_fn->prototype.param_count, 1);
  LLVMValueRef llvm_called_fn =
      LLVMAddFunction(llvm_module, called_fn->prototype.name, llvm_fn_type);

  LLVMValueRef llvm_args[call_expr.args.argc];

  for (size_t i = 0; i < call_expr.args.argc; ++i) {
    LLVMValueRef llvm_arg = llvm_emit_stmt_expr(call_expr.args.argv[i]);

    if (LLVMIsConstantString(llvm_arg)) {
      size_t str_len = 0;
      const char *str = LLVMGetAsString(llvm_arg, &str_len);
      llvm_args[i] = LLVMBuildGlobalString(llvm_builder, str, "str");
      continue;
    }

    if (LLVMIsAConstantInt(llvm_arg)) {
      llvm_args[i] = llvm_arg;
      continue;
    }

    fprintf(stderr, "[Error] Unsupported argument\n");
    exit(1);
  }

  LLVMValueRef llvm_call =
      LLVMBuildCall2(llvm_builder, llvm_fn_type, llvm_called_fn, llvm_args,
                     call_expr.args.argc, "");
  return llvm_call;
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
    return LLVMConstString(unescaped_str, strlen(unescaped_str), 0);
  }
  }
}

LLVMValueRef llvm_emit_expr_ident(ExprIdent ident) {
  return LLVMGetNamedGlobal(llvm_module, ident.label);
}

LLVMTypeRef sml_to_llvm_type(Type type) {
  switch (type) {
  case TYPE_INT:
    return LLVMInt32Type();
  case TYPE_STR:
    return LLVMPointerType(LLVMInt8Type(), 0);
  }
}
