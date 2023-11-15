#ifndef ELYSIA_PARSER_H_
#define ELYSIA_PARSER_H_

#include "plenary.h"
#include "compiler.h"
#include "lexer.h"
#include <stdint.h>

typedef enum {
    BINARY_OP_UNKNOWN = 0,
    BINARY_OP_ADD,
    BINARY_OP_SUB,
    BINARY_OP_DIV,
    BINARY_OP_MUL,
    BINARY_OP_MOD,
    BINARY_OP_EQ,
    BINARY_OP_NE,
    BINARY_OP_LT,
    BINARY_OP_LE,
    BINARY_OP_GT,
    BINARY_OP_GE,
    BINARY_OP_AND,
    BINARY_OP_OR,
    BINARY_OP_XOR,
    BINARY_OP_SHL,
    BINARY_OP_SHR,
} Binary_Op_Type;

typedef enum {
    EXPR_UNKNOWN = 0,
    EXPR_INTEGER_LITERAL,
    EXPR_STRING_LITERAL,
    EXPR_BOOL_LITERAL,
    EXPR_FLOAT_LITERAL,
    EXPR_FUNCALL,
    EXPR_VAR_READ,
    EXPR_BINARY_OP,
} Expr_Type;

typedef struct Expr Expr;
typedef da(Expr) Expr_List;
typedef struct Expr_Var_Read Expr_Var_Read;
typedef struct Expr_Binary_Op Expr_Binary_Op;
typedef struct Expr_Func_Call Expr_Func_Call;
typedef union Expr_As Expr_As;

struct Expr_Var_Read {
    Location loc;
    String_View name;
};

struct Expr_Func_Call {
    Location loc;
    Expr_List params;
};

union Expr_As {
    String_View literal_str;
    bool literal_bool;
    int64_t literal_int;

    Expr_Var_Read var_read;
    Expr_Func_Call func_call;
    Expr_Binary_Op *binop;
};

struct Expr {
    Location loc;
    Expr_Type type;
    Expr_As as;
};

struct Expr_Binary_Op {
    Location loc;
    Binary_Op_Type type;
    Expr left;
    Expr right;
};

typedef enum {
    STMT_UNKNOWN = 0,
    STMT_RETURN, STMT_IF, STMT_WHILE, STMT_FOR, STMT_VAR_ASSIGN, STMT_VAR_DEF, STMT_VAR_INIT, STMT_EXPR
} Stmt_Type;

typedef struct Stmt Stmt;
typedef da(Stmt) Block;
typedef struct Stmt_Return Stmt_Return;
typedef union Stmt_As Stmt_As;

struct Stmt_Return {
    Location loc;
    Expr value;
};

union Stmt_As {
    Stmt_Return _return;
    Expr expr;
};

struct Stmt {
    Location loc;
    Stmt_Type type;
    Stmt_As as;
};

typedef struct {
    Location loc;
    String_View name;
    String_View type_name;
} Func_Param;

typedef da(Func_Param) Func_Param_List;

typedef struct {
    Location loc;
    String_View name;
    Func_Param_List params;
    String_View return_type_name;
    Block body;
} Func_Def;

typedef struct {
    Func_Def main;
} Module;

Expr parse_expr(Lexer *lex);
Stmt parse_stmt(Lexer *lex);
Func_Param_List parse_func_params(Lexer *lex);
Block parse_block(Lexer *lex);
Func_Def parse_func_def(Lexer* lex);
Module parse_module(Lexer* lex);
Binary_Op_Type binary_op_type_from_token_type(Token_Type type);

#endif // ELYSIA_PARSER_H_
