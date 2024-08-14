#include "ast.h"
#include "type.h"

typedef struct TypeCheckContext {
  int error_count;
} TypeCheckContext;

int AST_type_check(AST *);
void type_check_stmt_block(TypeCheckContext *, StmtBlock *);
void type_check_stmt_vardecl(TypeCheckContext *, StmtVarDecl *);
Type expr_to_type(StmtExpr *);

int AST_type_check(AST *ast) {
  TypeCheckContext ctx;
  ctx.error_count = 0;

  type_check_stmt_block(&ctx, ast);

  return ctx.error_count;
}

void type_check_stmt_block(TypeCheckContext *ctx, StmtBlock *block) {
  for (size_t i = 0; i < block->stmt_count; ++i) {
    Stmt *stmt = &block->stmts[i];
    switch (stmt->type) {
    case STMT_VAR_DECL:
      type_check_stmt_vardecl(ctx, &stmt->value.var_decl);
      break;
    }
  }
}

void type_check_stmt_vardecl(TypeCheckContext *ctx, StmtVarDecl *var_decl) {
  Type var_type = expr_to_type(var_decl->init);
  var_decl->type = var_type;
}

Type expr_to_type(StmtExpr *expr) {
  switch (expr->type) {
  case EXPR_LITERAL:
    switch (expr->value.literal.type) {
    case EXPR_LITERAL_NUM:
      return TYPE_INT;
      break;
    case EXPR_LITERAL_STR:
      return TYPE_STR;
      break;
    }
    break;
  }
}
