#ifndef ELYSIA_PARSER_H_
#define ELYSIA_PARSER_H_

#include "sv.h"
#include "arena.h"
#include "elysia_lexer.h"
#include "elysia_types.h"

typedef enum {
    BINARY_OP_UNKNOWN = 0,
    BINARY_OP_ADD, BINARY_OP_SUB, BINARY_OP_DIV, BINARY_OP_MUL, BINARY_OP_MOD, BINARY_OP_EQ,
    BINARY_OP_NE, BINARY_OP_LT, BINARY_OP_LE, BINARY_OP_GT, BINARY_OP_GE, BINARY_OP_AND,
    BINARY_OP_OR, BINARY_OP_XOR, BINARY_OP_SHL, BINARY_OP_SHR,
} Binary_Op_Type;

typedef enum {
    EXPR_UNKNOWN = 0,
    EXPR_INTEGER_LITERAL, EXPR_STRING_LITERAL, EXPR_BOOL_LITERAL, EXPR_FLOAT_LITERAL,
    EXPR_TYPECAST, EXPR_FUNCALL, EXPR_VAR_READ, EXPR_BINARY_OP,

    COUNT_EXPRS,
} Expr_Type;

typedef struct Expr Expr;
typedef struct Expr_Type_Cast Expr_Type_Cast;
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
    String_View name;
    Location loc;
    Expr_List args;
};

union Expr_As {
    String_View literal_str;
    bool literal_bool;
    int64_t literal_int;
    double  literal_float;

    Expr_Var_Read var_read;
    Expr_Func_Call func_call;
    Expr_Binary_Op *binop;
    Expr_Type_Cast *type_cast;
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

struct Expr_Type_Cast {
    Data_Type into;
    Expr value;
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
typedef struct Stmt_If Stmt_If;

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
    Data_Type type;
};

struct Stmt_Var_Init {
    String_View name;
    Data_Type type;
    Expr value;
    bool infer_type;
};

struct Stmt_Var_Assign {
    String_View name;
    Expr value;
};

struct Stmt_While {
    Expr condition;
    Block todo;
};

// TODO (bagasjs): Maybe other methods other than linked list?
struct Stmt_If {
    Location loc;
    Expr condition;
    Block todo;
    Stmt_If *elif;
    Block _else;
};

union Stmt_As {
    Stmt_Return _return;
    Stmt_Var_Def var_def;
    Stmt_Var_Assign var_assign;
    Stmt_Var_Init var_init;
    Stmt_While _while;
    Stmt_If _if;
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
    Data_Type type;
} Func_Param;

typedef struct {
    Func_Param *data;
    size_t count, capacity;
} Func_Param_List;

typedef struct {
    Location loc;
    String_View name;
    Func_Param_List params;
    Data_Type return_type;
    Block body;
} Func_Def;

typedef struct {
    Func_Def *main;
    struct {
        Func_Def *data;
        size_t count;
        size_t capacity;
    } functions;
} Module;

void push_param_to_param_list(Arena *arena, Func_Param_List *params, Func_Param param);
void push_expr_to_expr_list(Arena *arena, Expr_List *list, Expr expr);
void push_stmt_to_block(Arena *arena, Block *block, Stmt stmt);
void push_fdef_to_module(Arena *arena, Module *module, Func_Def fdef);
Binary_Op_Type binary_op_type_from_token_type(Token_Type type);

Data_Type parse_data_type(Arena *arena, Lexer *lex);
Expr parse_expr(Arena *arena, Lexer *lex);
Stmt parse_stmt(Arena *arena, Lexer *lex);
Func_Param_List parse_func_params(Arena *arena, Lexer *lex);
Block parse_block(Arena *arena, Lexer *lex);
Func_Def parse_func_def(Arena *arena, Lexer* lex);
Module parse_module(Arena *arena, Lexer* lex);

void dump_func_def(const Func_Def *func_def, size_t depth);
void dump_stmt(const Stmt *stmt, size_t depth);
void dump_expr(const Expr *expr, size_t depth);

#endif // ELYSIA_PARSER_H_
