#ifndef ELYSIA_AST_H_
#define ELYSIA_AST_H_

#include "sv.h"
#include "arena.h"
#include "elysia_compiler.h"
#include "elysia_lexer.h"

typedef enum {
    BINARY_OP_UNKNOWN = 0,
    BINARY_OP_ADD, BINARY_OP_SUB, BINARY_OP_DIV, BINARY_OP_MUL, BINARY_OP_MOD, BINARY_OP_EQ,
    BINARY_OP_NE, BINARY_OP_LT, BINARY_OP_LE, BINARY_OP_GT, BINARY_OP_GE, BINARY_OP_AND,
    BINARY_OP_OR, BINARY_OP_XOR, BINARY_OP_SHL, BINARY_OP_SHR,
} Binary_Op_Type;

typedef struct {
    String_View name;
    bool is_ptr;
    bool is_array;
    size_t array_len;
} Parsed_Data_Type;

typedef enum {
    EXPR_UNKNOWN = 0,
    EXPR_INTEGER_LITERAL, EXPR_STRING_LITERAL, EXPR_BOOL_LITERAL, EXPR_FLOAT_LITERAL,
    EXPR_FUNCALL, EXPR_VAR_READ, EXPR_BINARY_OP,

    COUNT_EXPRS,
} Expr_Type;

typedef struct Expr Expr;
typedef struct Expr_Var_Read Expr_Var_Read;
typedef struct Expr_Binary_Op Expr_Binary_Op;
typedef struct Expr_Func_Call Expr_Func_Call;
typedef union Expr_As Expr_As;

typedef struct {
    Expr *data;
    size_t count, capacity;
} Expr_List;

struct Expr_Var_Read {
    Location loc;
    String_View name;
};

struct Expr_Func_Call {
    Location loc;
    Expr_List args;
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
    STMT_RETURN, STMT_VAR_ASSIGN, STMT_VAR_DEF, STMT_VAR_INIT, STMT_EXPR,
    STMT_IF, STMT_WHILE,

    COUNT_STMTS,
} Stmt_Type;

typedef struct Stmt Stmt;
typedef struct Stmt_Return Stmt_Return;
typedef struct Stmt_Var_Def Stmt_Var_Def;
typedef struct Stmt_Var_Init Stmt_Var_Init;
typedef struct Stmt_Var_Assign Stmt_Var_Assign;
typedef struct Stmt_While Stmt_While;
typedef union Stmt_As Stmt_As;
typedef struct {
    Stmt *data;
    size_t count, capacity;
} Block;

struct Stmt_Return {
    Location loc;
    Expr value;
};

struct Stmt_Var_Def {
    String_View name;
    Parsed_Data_Type type;
};

struct Stmt_Var_Init {
    String_View name;
    Parsed_Data_Type type;
    Expr value;
};

struct Stmt_Var_Assign {
    String_View name;
    Expr value;
};

struct Stmt_While {
    Expr condition;
    Block todo;
};

union Stmt_As {
    Stmt_Return _return;
    Stmt_Var_Def var_def;
    Stmt_Var_Assign var_assign;
    Stmt_Var_Init var_init;
    Stmt_While _while;
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
    Parsed_Data_Type type;
} Func_Param;

typedef struct {
    Func_Param *data;
    size_t count, capacity;
} Func_Param_List;

typedef struct {
    Location loc;
    String_View name;
    Func_Param_List params;
    Parsed_Data_Type return_type;
    Block body;
} Func_Def;

typedef struct {
    Func_Def main;
} Module;

void push_param_to_param_list(Arena *arena, Func_Param_List *params, Func_Param param);
void push_expr_to_expr_list(Arena *arena, Expr_List *list, Expr expr);
void push_stmt_to_block(Arena *arena, Block *block, Stmt stmt);
Binary_Op_Type binary_op_type_from_token_type(Token_Type type);

void dump_func_def(const Func_Def *func_def, size_t depth);
void dump_stmt(const Stmt *stmt, size_t depth);
void dump_expr(const Expr *expr, size_t depth);
void dump_parsed_type(const Parsed_Data_Type *type);

#endif // ELYSIA_AST_H_
