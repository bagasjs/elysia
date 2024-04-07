#ifndef ELYSIA_COMPILER_H_
#define ELYSIA_COMPILER_H_

#include "elysia.h"
#include "elysia_ast.h"

#define ELYSIA_SCOPE_VARS_CAPACITY 1024
#define ELYSIA_MODULE_FUNCTIONS_CAPACITY 1024

typedef struct Jump_Target Jump_Target;
struct Jump_Target {
    int kind;
    Jump_Target *begin;
    Jump_Target *next;
    Jump_Target *prev;
    Jump_Target *end;
};

typedef struct {
    String_View name;
    size_t address;
    Data_Type type;
} Evaluated_Var;

typedef struct Scope Scope;
struct Scope {
    Scope *parent;
    struct {
        Evaluated_Var data[ELYSIA_SCOPE_VARS_CAPACITY];
        uint32_t count;
    } vars;
    size_t stack_usage;
};

typedef struct Evaluated_Fn {
    Func_Def def;
    Scope scope;
    bool has_return_stmt;
    struct {
        Jump_Target *items;
        size_t count;
        size_t capacity;
    } targets;
} Evaluated_Fn;

typedef struct Evaluated_Module {
    Scope global;
    struct {
        Evaluated_Fn data[ELYSIA_MODULE_FUNCTIONS_CAPACITY];
        uint32_t count;
    } functions;
} Evaluated_Module;

const Evaluated_Var *get_var_from_scope(const Scope *scope, String_View name);
bool emplace_var_to_scope(Scope *scope, String_View name, Data_Type type, size_t address);
bool push_var_to_scope(Scope *scope, const Evaluated_Var var);

bool push_fn_to_module(Evaluated_Module *module, const Evaluated_Fn fn);
bool emplace_fn_to_module(Evaluated_Module *module, const Func_Def def);

Data_Type eval_expr(Evaluated_Module *module, const Scope *scope, const Expr *expr);
void eval_stmt(Evaluated_Module *module, Evaluated_Fn *fn, Scope *scope, const Stmt stmt);
void eval_func_def(Evaluated_Module *module, const Func_Def fdef);
bool eval_module(Evaluated_Module *result, const Module *module);

void compile_module_to_file(const char *file_path, Evaluated_Module *module);

#endif // ELYSIA_COMPILER_H_
