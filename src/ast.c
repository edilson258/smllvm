#include <stdio.h>

#include "ast.h"
#include "type.h"

typedef struct InspectContext {
  FILE *file;
  int tab;
  int tab_rate;
} InspectContext;

void inspect_write(InspectContext *ctx, char *text, ...);
void inspect_writeln(InspectContext *ctx, char *text, ...);

void AST_Inspect(AST ast);
void insect_stmt_block(InspectContext *, StmtBlock);
void insect_stmt_vardecl(InspectContext *, StmtVarDecl);
void insect_stmt_function(InspectContext *, StmtFnDecl);
void insect_stmt_return(InspectContext *, StmtReturn);
void insect_stmt_expr(InspectContext *, StmtExpr);
void inspect_expr_literal(InspectContext *, ExprLiteral);
void inspect_expr_call(InspectContext *, ExprCall);
void inspect_expr_binop(InspectContext *, ExprBinOp);

void AST_Inspect(AST ast) {
  InspectContext ctx;
  ctx.file = stdout;
  ctx.tab = 0;
  ctx.tab_rate = 4;

  insect_stmt_block(&ctx, ast);
}

void insect_stmt_block(InspectContext *ctx, StmtBlock block) {
  for (size_t i = 0; i < block.stmt_count; ++i) {
    Stmt stmt = block.stmts[i];
    switch (stmt.type) {
    case STMT_VAR_DECL:
      insect_stmt_vardecl(ctx, stmt.value.var_decl);
      break;
    case STMT_FN_DECL:
      insect_stmt_function(ctx, stmt.value.fn_decl);
      break;
    case STMT_EXPR:
      insect_stmt_expr(ctx, stmt.value.expr);
      break;
    case STMT_RETURN:
      insect_stmt_return(ctx, stmt.value.return_);
      break;
    }
  }
}

void insect_stmt_vardecl(InspectContext *ctx, StmtVarDecl var_decl) {
  inspect_writeln(ctx, "VARIABLE DECLARATION:");
  ctx->tab += ctx->tab_rate;
  inspect_writeln(ctx, "NAME: \"%s\"", var_decl.name);
  inspect_writeln(ctx, "TYPE: %s", TYPE(var_decl.type));
  inspect_write(ctx, "INIT:\n");
  ctx->tab += ctx->tab_rate;
  insect_stmt_expr(ctx, *var_decl.init);
  ctx->tab -= (ctx->tab_rate * 2);
}

void insect_stmt_function(InspectContext *ctx, StmtFnDecl fn) {
  inspect_writeln(ctx, "FUNCTION DECLARATION:");
  ctx->tab += ctx->tab_rate;
  inspect_writeln(ctx, "NAME: \"%s\"", fn.name);
  inspect_writeln(ctx, "RETURN TYPE: %s", TYPE(fn.return_type));
  inspect_writeln(ctx, "BODY:");
  ctx->tab += ctx->tab_rate;
  insect_stmt_block(ctx, fn.body);
  ctx->tab -= (ctx->tab_rate * 2);
}

void insect_stmt_return(InspectContext *ctx, StmtReturn ret) {
  inspect_writeln(ctx, "RETURN STATEMENT:");
  ctx->tab += ctx->tab_rate;
  insect_stmt_expr(ctx, ret.operand);
  ctx->tab -= ctx->tab_rate;
}

void insect_stmt_expr(InspectContext *ctx, StmtExpr expr) {
  switch (expr.type) {
  case EXPR_LITERAL:
    inspect_expr_literal(ctx, expr.value.literal);
    break;
  case EXPR_CALL:
    inspect_expr_call(ctx, expr.value.call);
    break;
  case EXPR_BINOP:
    inspect_expr_binop(ctx, expr.value.binop);
    break;
  case EXPR_IDENT:
    puts("[WARNING] couldn't inspect EXPR_IDENT");
    break;
  }
}

void inspect_expr_literal(InspectContext *ctx, ExprLiteral literal) {
  switch (literal.type) {
  case EXPR_LITERAL_NUM:
    inspect_writeln(ctx, "LITERAL(%lld)", literal.value.number);
    break;
  case EXPR_LITERAL_STR:
    inspect_writeln(ctx, "LITERAL(\"%s\")", literal.value.string);
    break;
  }
}

void inspect_expr_call(InspectContext *ctx, ExprCall call) {
  inspect_writeln(ctx, "FUNCTION CALL:");
  ctx->tab += ctx->tab_rate;
  inspect_writeln(ctx, "NAME: \"%s\"", call.name);

  inspect_writeln(ctx, "ARGS: [");
  ctx->tab += ctx->tab_rate;
  for (size_t i = 0; i < call.args.argc; ++i) {
    insect_stmt_expr(ctx, call.args.argv[i]);
  }
  ctx->tab -= ctx->tab_rate;
  inspect_writeln(ctx, "]");
  ctx->tab -= ctx->tab_rate;
}

void inspect_expr_binop(InspectContext *ctx, ExprBinOp binop) {
  inspect_writeln(ctx, "BINARY EXPRESSION:");
  ctx->tab += ctx->tab_rate;
  insect_stmt_expr(ctx, *binop.lhs);
  switch (binop.op) {
  case BINOP_PLUS:
    inspect_writeln(ctx, "+");
    break;
  }
  insect_stmt_expr(ctx, *binop.rhs);
  ctx->tab -= ctx->tab_rate;
}

void inspect_writeln(InspectContext *ctx, char *f, ...) {
  for (int i = 0; i < ctx->tab; ++i) {
    fprintf(ctx->file, " ");
  }
  va_list args;
  va_start(args, f);
  vfprintf(ctx->file, f, args);
  va_end(args);
  fprintf(ctx->file, "\n");
}

void inspect_write(InspectContext *ctx, char *f, ...) {
  for (int i = 0; i < ctx->tab; ++i) {
    fprintf(ctx->file, " ");
  }
  va_list args;
  va_start(args, f);
  vfprintf(ctx->file, f, args);
  va_end(args);
}
