#ifndef ELYSIA_COMPILER_H_
#define ELYSIA_COMPILER_H_

#include "elysia.h"
#include "elysia_ast.h"

#define ELYSIA_SCOPE_VARS_CAPACITY 1024
#define ELYSIA_MODULE_FUNCTIONS_CAPACITY 1024

typedef struct {
    String_View name;
    size_t index;
    Data_Type type;
} Compiled_Var;

typedef struct Scope Scope;
struct Scope {
    Scope *parent;
    struct {
        Compiled_Var data[ELYSIA_SCOPE_VARS_CAPACITY];
        uint32_t count;
    } vars;
};

typedef struct Compiled_Fn {
    Func_Def def;
    Scope scope;
} Compiled_Fn;

typedef struct Compiled_Module {
    Scope global;
    struct {
        Compiled_Fn data[ELYSIA_MODULE_FUNCTIONS_CAPACITY];
        uint32_t count;
    } functions;
} Compiled_Module;

Compiled_Var get_var_from_scope(Scope *scope, String_View name);

bool eval_module(Compiled_Module *result, const Module *module);
bool push_var_to_scope(Scope *scope, const Compiled_Var var);
bool push_fn_to_module(Compiled_Module *module, const Compiled_Fn fn);
bool emplace_fn_to_module(Compiled_Module *module, const Func_Def def);
bool emplace_var_to_scope(Scope *scope, String_View name, Data_Type type);

void compile_into_x86_64_nasm(const char *file_path, Compiled_Module *module);
Data_Type eval_expr(const Expr *expr);

#endif // ELYSIA_COMPILER_H_
