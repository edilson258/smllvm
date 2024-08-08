#include <stdio.h>

#include "ast.h"

#define TAB_RATE 4

void print_tab(int x) {
  for (int i = 0; i <= x; ++i) {
    printf(" ");
  }
}

void block_stmt_inspect(StmtBlock block, int tab);

void inspect_fn_decl(StmtFnDecl fn_decl, int tab) {
  printf("FUNCTION DECLARATION:\n");
  print_tab(tab);
  printf("NAME: '%s'\n", fn_decl.name);
  print_tab(tab);
  printf("RETURN TYPE: %s\n", TYPE(fn_decl.return_type));
  print_tab(tab);
  printf("BODY:\n");
  block_stmt_inspect(fn_decl.body, tab + TAB_RATE);
}

void inspect_expr(StmtExpr expr, int tab);

void inspect_expr_call(ExprCall call, int tab) {
  print_tab(tab);
  printf("CALL EXPR:\n");
  tab += TAB_RATE;
  print_tab(tab);
  printf("CALLEE: '%s'\n", call.callee);
  print_tab(tab);
  printf("ARGS: [\n");
  for (int i = 0; i < call.args.argc; ++i) {
    inspect_expr(call.args.argv[i], tab + TAB_RATE);
  }
  print_tab(tab);
  printf("]\n");
}

void inspect_literal(ExprLiteral literal, int tab) {
  switch (literal.type) {
  case EXPR_LITERAL_NUM:
    print_tab(tab);
    printf("LITERAL: %lld\n", literal.value.number);
    break;
  case EXPR_LITERAL_STR:
    print_tab(tab);
    printf("LITERAL: '%s'\n", literal.value.string);
    break;
  }
}

void inspect_expr(StmtExpr expr, int tab) {
  switch (expr.type) {
  case EXPR_CALL:
    inspect_expr_call(expr.value.call, tab);
    break;
  case EXPR_LITERAL:
    inspect_literal(expr.value.literal, tab);
    break;
  case EXPR_IDENT:
    print_tab(tab);
    printf("IDENTIIFIER: %s\n", expr.value.ident.label);
    break;
  }
}

void block_stmt_inspect(StmtBlock block, int tab) {
  for (size_t i = 0; i < block.stmt_count; ++i) {
    switch (block.stmts[i].type) {
    case STMT_EXPR:
      inspect_expr(block.stmts[i].value.expr, tab);
      break;
    case STMT_FN_DECL:
      inspect_fn_decl(block.stmts[i].value.fn_decl, tab);
      break;
    case STMT_RETURN:
      print_tab(tab);
      printf("RETURN:\n");
      inspect_expr(block.stmts[i].value.return_.operand, tab + TAB_RATE);
      break;
    }
  }
}

void AST_Inspect(AST ast) { block_stmt_inspect(ast, TAB_RATE); }
