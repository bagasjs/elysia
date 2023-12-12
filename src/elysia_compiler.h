#ifndef ELYSIA_COMPILER_H_
#define ELYSIA_COMPILER_H_

#include "elysia.h"
#include "elysia_ast.h"

#define ELYSIA_SCOPE_VARS_CAPACITY 1024

typedef struct {
    const char *name;
    Data_Type type;
} Compiled_Var;

typedef struct Scope Scope;
struct Scope {
    Scope *parent;
    Compiled_Var var[ELYSIA_SCOPE_VARS_CAPACITY];
    size_t count;
};

typedef struct Compiled_Fn {
    Func_Def def;
    Scope scope;
} Compiled_Fn;

typedef struct {
    Scope *scope;
} Generator;

Compiled_Var get_var_from_scope(Scope *scope, String_View name);
bool push_var_to_scope(Scope *scope, Compiled_Var var);

void compile_into_x86_64_nasm(const char *file_path, const Module *module);
Data_Type eval_expr(const Expr *expr);

#endif // ELYSIA_COMPILER_H_
