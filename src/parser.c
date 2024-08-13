#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "token.h"

typedef enum Precedence {
  PRECEDENCE_LOWEST = 1,
  PRECEDENCE_ADDITIVE = 2,
  PRECEDENCE_CALL = 3,
} Precedence;

static inline void bump(Parser *p);
static inline void bump_expexted(Parser *p, TokenType);
Stmt parse_stmt(Parser *);
StmtFnDecl parse_stmt_fndecl(Parser *);
StmtReturn parse_stmt_return(Parser *);
StmtBlock parse_stmt_block(Parser *);
StmtExpr parse_expr(Parser *, Precedence);
ExprCall parse_expr_call(Parser *, StmtExpr);
ExprCallArgs parse_expr_call_args(Parser *);
ExprBinOp parse_expr_binop(Parser *, StmtExpr);
Precedence token_to_precedence(TokenType);

Parser Parser_New(Lexer lexer) {
  Parser parser;
  parser.lexer = lexer;
  return parser;
}

AST Parse(Parser *p) {
  bump(p);
  bump(p);

  AST block;
  block.stmt_count = 0;
  block.capacity = SML_BLOCK_STMT_CAP;
  block.stmts = malloc(sizeof(Stmt) * SML_BLOCK_STMT_CAP);

  while (p->curr_token.type != TOKEN_EOF) {
    Stmt stmt = parse_stmt(p);
    block.stmts[block.stmt_count++] = stmt;
  }

  return block;
}

Stmt parse_stmt(Parser *p) {
  switch (p->curr_token.type) {
  case TOKEN_FN_DECL: {
    Stmt stmt = {.type = STMT_FN_DECL, .value.fn_decl = parse_stmt_fndecl(p)};
    return stmt;
  }
  case TOKEN_RETURN: {
    Stmt stmt = {.type = STMT_RETURN, .value.return_ = parse_stmt_return(p)};
    return stmt;
  }
  default: {
    Stmt stmt = {.type = STMT_EXPR,
                 .value.expr = parse_expr(p, PRECEDENCE_LOWEST)};
    bump_expexted(p, TOKEN_SEMICOLON);
    return stmt;
  }
  }
}

StmtFnDecl parse_stmt_fndecl(Parser *p) {
  bump(p);
  if (p->curr_token.type != TOKEN_IDENT) {
    puts("Expected idenifier after 'function' but got: ");
    Token_Inspect(&p->curr_token);
    exit(1);
  }

  char *fn_name = p->curr_token.value.string;
  bump(p);

  bump_expexted(p, TOKEN_LPAREN);
  // handle params
  bump_expexted(p, TOKEN_RPAREN);
  bump_expexted(p, TOKEN_ARROW);

  Type return_type;
  switch (p->curr_token.type) {
  case TOKEN_TYPE_INT:
    return_type = TYPE_INT;
    break;
  default:
    puts("Expected type but got: ");
    Token_Inspect(&p->curr_token);
    exit(1);
  }
  bump(p);

  StmtFnDecl fn = {
      .name = fn_name, .body = parse_stmt_block(p), .return_type = return_type};
  return fn;
}

StmtBlock parse_stmt_block(Parser *p) {
  StmtBlock block;
  block.stmt_count = 0;
  block.capacity = SML_BLOCK_STMT_CAP;
  block.stmts = malloc(sizeof(Stmt) * SML_BLOCK_STMT_CAP);

  bump_expexted(p, TOKEN_LBRACE);

  while (p->curr_token.type != TOKEN_EOF &&
         p->curr_token.type != TOKEN_RBRACE) {
    Stmt stmt = parse_stmt(p);
    block.stmts[block.stmt_count++] = stmt;
  }

  bump_expexted(p, TOKEN_RBRACE);

  return block;
}

StmtReturn parse_stmt_return(Parser *p) {
  bump(p); // eat 'return'

  StmtReturn stmt_ret;
  stmt_ret.operand = parse_expr(p, PRECEDENCE_LOWEST);
  bump_expexted(p, TOKEN_SEMICOLON);

  return stmt_ret;
}

StmtExpr parse_expr(Parser *p, Precedence precedence) {
  StmtExpr lhs;
  switch (p->curr_token.type) {
  case TOKEN_IDENT:
    lhs.type = EXPR_IDENT;
    lhs.value.ident.label = p->curr_token.value.string;
    break;
  case TOKEN_STRING:
    lhs.type = EXPR_LITERAL;
    lhs.value.literal.type = EXPR_LITERAL_STR;
    lhs.value.literal.value.string = p->curr_token.value.string;
    break;
  case TOKEN_NUMBER:
    lhs.type = EXPR_LITERAL;
    lhs.value.literal.type = EXPR_LITERAL_NUM;
    lhs.value.literal.value.number = p->curr_token.value.number;
    break;
  default:
    puts("parse_expr: Unexpected token: ");
    Token_Inspect(&p->curr_token);
    exit(1);
  }
  bump(p);

  while (p->curr_token.type != TOKEN_EOF &&
         precedence < token_to_precedence(p->curr_token.type)) {
    switch (p->curr_token.type) {
    case TOKEN_LPAREN: {
      StmtExpr expr = {.type = EXPR_CALL,
                       .value.call = parse_expr_call(p, lhs)};
      lhs = expr;
      break;
    }
    case TOKEN_PLUS: {
      StmtExpr expr = {.type = EXPR_BINOP,
                       .value.binop = parse_expr_binop(p, lhs)};
      lhs = expr;
      break;
    }
    default:
      return lhs;
    }
  }

  return lhs;
}

ExprCall parse_expr_call(Parser *p, StmtExpr lhs) {
  if (lhs.type != EXPR_IDENT) {
    puts("Invalid call expr");
    exit(1);
  }
  ExprCallArgs args = parse_expr_call_args(p);
  ExprCall call = {.callee = lhs.value.ident.label, .args = args};
  return call;
}

ExprCallArgs parse_expr_call_args(Parser *p) {
  bump_expexted(p, TOKEN_LPAREN);

  ExprCallArgs args;
  args.argc = 0;
  args.capacity = SML_CALL_ARGS_CAP;
  args.argv = malloc(sizeof(StmtExpr) * SML_CALL_ARGS_CAP);

  while (p->curr_token.type != TOKEN_EOF &&
         p->curr_token.type != TOKEN_RPAREN) {
    StmtExpr arg = parse_expr(p, PRECEDENCE_CALL);
    memmove(args.argv + args.argc++, &arg, sizeof(StmtExpr));
  }

  bump_expexted(p, TOKEN_RPAREN);

  return args;
}

ExprBinOp parse_expr_binop(Parser *p, StmtExpr lhs) {
  BinOperator op;

  switch (p->curr_token.type) {
  case TOKEN_PLUS:
    op = BINOP_PLUS;
    break;
  default:
    fprintf(stderr, "Invalid binop\n");
    exit(1);
  }
  TokenType op_type = p->curr_token.type;
  bump(p);

  ExprBinOp binop;

  binop.op = op;
  binop.lhs = malloc(sizeof(ExprBinOp));
  binop.rhs = malloc(sizeof(ExprBinOp));

  memmove(binop.lhs, &lhs, sizeof(StmtExpr));
  StmtExpr rhs = parse_expr(p, token_to_precedence(op_type));
  memmove(binop.rhs, &rhs, sizeof(StmtExpr));

  return binop;
}

static inline void bump(Parser *p) {
  p->curr_token = p->next_token;
  p->next_token = Lexer_NextToken(&p->lexer);
}

static inline void bump_expexted(Parser *p, TokenType expected) {
  if (p->curr_token.type == expected) {
    return bump(p);
  }
  puts("bump_expexted: Unexpecte token: ");
  Token_Inspect(&p->curr_token);
  exit(1);
}

Precedence token_to_precedence(TokenType tt) {
  switch (tt) {
  case TOKEN_LPAREN:
    return PRECEDENCE_CALL;
  case TOKEN_PLUS:
    return PRECEDENCE_ADDITIVE;
  default:
    return PRECEDENCE_LOWEST;
  }
}
