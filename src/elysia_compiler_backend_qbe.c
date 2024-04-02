#include "elysia.h"
#include "elysia_ast.h"
#include "elysia_compiler.h"
#include "elysia_types.h"
#include <stdio.h>
#include <stdlib.h>

static void compile_expr_into_qbe(FILE *f, Evaluated_Module *module, Scope *scope, const Expr expr)
{
    switch(expr.type) {
        case EXPR_INTEGER_LITERAL:
            {
                fprintf(f, "    %%_1 =w copy %ld # %s:%d\n", expr.as.literal_int, __FILE__, __LINE__);
            } break;
        case EXPR_FUNCALL:
            {
                fprintf(f, "    call "SV_FMT"()\n", SV_ARGV(expr.as.func_call.name));
            } break;
        case EXPR_VAR_READ:
            {
                const Evaluated_Var *var = get_var_from_scope(scope, expr.as.var_read.name);
                fprintf(f, "    %%_1 =w copy %%"SV_FMT" # %s:%d\n", SV_ARGV(var->name), __FILE__, __LINE__);
            } break;
        case EXPR_BINARY_OP:
            {
                compile_expr_into_qbe(f, module, scope, expr.as.binop->right);
                fprintf(f, "    %%_2 =w copy %%_1 # %s:%d\n", __FILE__, __LINE__);
                compile_expr_into_qbe(f, module, scope, expr.as.binop->left);
                switch(expr.as.binop->type) {
                    case BINARY_OP_ADD:
                        {
                            fprintf(f, "    %%_1 =w add %%_1, %%_2 # %s:%d\n", __FILE__, __LINE__);
                        } break;
                    case BINARY_OP_SUB:
                        {
                            fprintf(f, "    %%_1 =w sub %%_1, %%_2 # %s:%d\n", __FILE__, __LINE__);
                        } break;
                    case BINARY_OP_MUL:
                        {
                            fprintf(f, "    %%_1 =w mul %%_1, %%_2 # %s:%d\n", __FILE__, __LINE__);
                        } break;
                    default:
                        {
                            compilation_error(expr.loc, "Parsed but not implemented expression\n");
                            compilation_failure();
                        } break;
                }
            } break;
        default:
            {
                compilation_error(expr.loc, "Unreachable expression type");
                compilation_failure();
            } break;
    }
}

static void compile_stmt_into_qbe(FILE *f, Evaluated_Module *module, Evaluated_Fn *fn, Scope *scope, const Stmt stmt)
{
    switch(stmt.type) {
        case STMT_VAR_DEF:
            {
            } break;
        case STMT_VAR_INIT:
            {
                compile_expr_into_qbe(f, module, scope, stmt.as.var_init.value);
                fprintf(f, "    %%"SV_FMT" =w copy %%_1 # %s:%d\n", SV_ARGV(stmt.as.var_init.name), __FILE__, __LINE__);
            } break;
        case STMT_VAR_ASSIGN:
            {
                compile_expr_into_qbe(f, module, scope, stmt.as.var_assign.value);
                fprintf(f, "    %%"SV_FMT" =w copy %%_1 # %s:%d\n", SV_ARGV(stmt.as.var_init.name), __FILE__, __LINE__);
            } break;
        case STMT_RETURN:
            {
                compile_expr_into_qbe(f, module, scope, stmt.as._return.value);
                fprintf(f, "    ret %%_1\n}\n");
            } break;
        case STMT_WHILE:
            {
                fprintf(f, "@L1b");
                compile_expr_into_qbe(f, module, scope, stmt.as._while.condition);
                fprintf(f, "jz %%_1, @L1b, @L1e");
                for(size_t i = 0; i < stmt.as._while.todo.count; ++i) 
                    compile_stmt_into_qbe(f, module, fn, &fn->scope, stmt.as._while.todo.data[i]);
                fprintf(f, "@L1e");
            } break;
        default:
            {
                fatal("Unreachable");
            } break;
    }
}

static void compile_func_def_into_qbe(Evaluated_Module *module, FILE *f, Evaluated_Fn *fn)
{
    fprintf(f, "export function w $"SV_FMT"() {\n", SV_ARGV(fn->def.name));
    fprintf(f, "@start\n");
    for(size_t i = 0; i < fn->def.body.count; ++i) 
        compile_stmt_into_qbe(f, module, fn, &fn->scope, fn->def.body.data[i]);
    if(!fn->has_return_stmt) 
        fprintf(f, "   ret\n}\n");
}

void compile_module_to_file(const char *file_path, Evaluated_Module *module)
{
    FILE *f = fopen(file_path, "w");
    if(!f) {
        fatal("Failed to open file file %s", file_path);
    }

    for(uint32_t i = 0; i < module->functions.count; ++i) {
        compile_func_def_into_qbe(module, f, &module->functions.data[i]);
    }
}
