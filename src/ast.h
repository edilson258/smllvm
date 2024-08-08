#ifndef SML_AST
#define SML_AST

#include <stddef.h>

#include "type.h"

#define SML_BLOCK_STMT_CAP 25
#define SML_CALL_ARGS_CAP 25

typedef enum StmtType {
  STMT_FN_DECL = 1,
  STMT_RETURN,
  STMT_EXPR,
} StmtType;

typedef enum ExprType {
  EXPR_CALL = 1,
  EXPR_IDENT,
  EXPR_LITERAL,
} ExprType;

typedef enum ExprLiteralType {
  EXPR_LITERAL_STR = 1,
  EXPR_LITERAL_NUM,
} ExprLiteralType;

/*  Exprs  */

typedef struct ExprCallArgs {
  size_t argc;
  size_t capacity;
  struct StmtExpr *argv;
} ExprCallArgs;

typedef struct ExprCall {
  char *callee;
  ExprCallArgs args;
} ExprCall;

typedef struct ExprIdent {
  char *label;
} ExprIdent;

typedef union ExprLiteralValue {
  char *string;
  long long number;
} ExprLiteralValue;

typedef struct ExprLiteral {
  ExprLiteralType type;
  ExprLiteralValue value;
} ExprLiteral;

typedef union ExprValue {
  ExprCall call;
  ExprIdent ident;
  ExprLiteral literal;
} ExprValue;

typedef struct StmtExpr {
  ExprType type;
  ExprValue value;
} StmtExpr;

/*  Stmts  */

typedef struct StmtBlock {
  struct Stmt *stmts;
  size_t capacity;
  size_t stmt_count;
} StmtBlock;

typedef struct ReturnStmt {
  StmtExpr operand;
} StmtReturn;

typedef struct StmtFnDecl {
  char *name;
  StmtBlock body;
  Type return_type;
} StmtFnDecl;

typedef union StmtValue {
  StmtExpr expr;
  StmtFnDecl fn_decl;
  StmtReturn return_;
} StmtValue;

typedef struct Stmt {
  StmtType type;
  StmtValue value;
} Stmt;

typedef StmtBlock AST;

void AST_Inspect(AST ast);

#endif
