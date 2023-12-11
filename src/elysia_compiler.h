#ifndef ELYSIA_COMPILER_H_
#define ELYSIA_COMPILER_H_

#include "elysia.h"
#include "elysia_ast.h"

#define ELYSIA_SCOPE_VARS_CAPACITY 1024

typedef struct {
    String_View name;
    bool is_ptr;
    bool is_array;
    size_t size;
    size_t array_len;
} Data_Type;

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

typedef struct {
    Scope *scope;
} Compiler;

Data_Type data_type_from_parsed_type(Parsed_Type type);
Compiled_Var get_var_from_scope(Scope *scope, String_View name);
bool push_var_to_scope(Scope *scope, Compiled_Var var);

#endif // ELYSIA_COMPILER_H_
