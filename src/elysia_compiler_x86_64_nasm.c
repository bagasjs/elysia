#include "elysia.h"
#include "elysia_ast.h"
#include "elysia_compiler.h"
#include "elysia_types.h"
#include <stdio.h>
#include <stdlib.h>

static void compile_expr_into_x86_64_nasm(Compiled_Module *module, FILE *f, Scope *scope, const Expr expr)
{
    switch(expr.type) {
        case EXPR_INTEGER_LITERAL:
            {
                fprintf(f, "    mov rax, %ld\n", expr.as.literal_int);
            } break;
        case EXPR_FUNCALL:
            {
                fprintf(f, "    call "SV_FMT"\n", SV_ARGV(expr.as.func_call.name));
            } break;
        case EXPR_VAR_READ:
            {
                const Compiled_Var *var = get_var_from_scope(scope, expr.as.var_read.name);
                fprintf(f, "    mov rax, QWORD[rbp-%zu]\n", var->address);
            } break;
        case EXPR_BINARY_OP:
            {
                compile_expr_into_x86_64_nasm(module, f, scope, expr.as.binop->right);
                fprintf(f, "    mov rbx, rax\n");
                compile_expr_into_x86_64_nasm(module, f, scope, expr.as.binop->left);
                switch(expr.as.binop->type) {
                    case BINARY_OP_ADD:
                        {
                            fprintf(f, "    add rax, rbx\n");
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

static void compile_stmt_into_x86_64_nasm(Compiled_Module *module, FILE *f, Scope *scope, const Stmt stmt)
{
    switch(stmt.type) {
        case STMT_VAR_DEF:
            {
                Data_Type data_type = stmt.as.var_def.type;
                emplace_var_to_scope(scope, stmt.as.var_def.name, data_type, scope->stack_usage);
                scope->stack_usage += get_data_type_size(&data_type);
            } break;
        case STMT_VAR_INIT:
            {
                size_t addr = scope->stack_usage;
                Data_Type variable_type = eval_expr(scope, &stmt.as.var_init.value);
                if(!stmt.as.var_init.infer_type) {
                    if(compare_data_type(&variable_type, &stmt.as.var_init.type) != DATA_TYPE_CMP_EQUAL) {
                        compilation_error(stmt.loc, "Variable initialization with name `"
                                SV_FMT"` expecting value with type `",
                                SV_ARGV(stmt.as.var_init.name));
                        dump_data_type(stderr, &stmt.as.var_init.type);
                        fprintf(stderr, "` but found `");
                        dump_data_type(stderr, &variable_type);
                    } else {
                        variable_type = stmt.as.var_init.type;
                    }
                }
                size_t variable_size = get_data_type_size(&variable_type);
                scope->stack_usage += variable_size;
                // printf("Variable "SV_FMT" with size "SV_FMT" with size %zu\n", SV_ARGV(stmt.as.var_init.name), SV_ARGV(variable_type.name), variable_size);
                emplace_var_to_scope(scope, stmt.as.var_init.name, variable_type, addr);

                compile_expr_into_x86_64_nasm(module, f, scope, stmt.as.var_init.value);
                fprintf(f, "    mov QWORD[rbp-%zu], rax\n", addr);
            } break;
        case STMT_VAR_ASSIGN:
            {
                const Compiled_Var *var = get_var_from_scope(scope, stmt.as.var_assign.name);
                Data_Type variable_type = eval_expr(scope, &stmt.as.var_assign.value);
                if(compare_data_type(&variable_type, &stmt.as.var_init.type) != DATA_TYPE_CMP_EQUAL) {
                    compilation_type_error(stmt.loc, &var->type, &variable_type, " while assigning value to variable "SV_FMT, 
                            SV_ARGV(stmt.as.var_assign.name));
                    compilation_error(stmt.loc, "Variable assignment to `"SV_FMT"` expecting value with type `",
                            SV_ARGV(stmt.as.var_assign.name));
                    dump_data_type(stderr, &var->type);
                    fprintf(stderr, "` but found `");
                    dump_data_type(stderr, &variable_type);
                }
                compile_expr_into_x86_64_nasm(module, f, scope, stmt.as.var_assign.value);
                fprintf(f, "    mov QWORD[rbp-%zu], rax\n", var->address);
            } break;
        case STMT_RETURN:
            {
                // Nothing will be handled by compile_func_def_into_x86_64_nasm
            } break;
        default:
            {
                fatal("Unreachable");
            } break;
    }
}

static void compile_func_def_into_x86_64_nasm(Compiled_Module *module, FILE *f, Compiled_Fn *fn)
{
    fprintf(f, SV_FMT":\n", SV_ARGV(fn->def.name));
    fprintf(f, "    push rbp\n");
    fprintf(f, "    mov rbp, rsp\n");
    fn->scope.stack_usage += 8;
    for(size_t i = 0; i < fn->def.body.count; ++i) {
        Stmt stmt = fn->def.body.data[i];
        compile_stmt_into_x86_64_nasm(module, f, &fn->scope, stmt);
        if(fn->def.body.data[i].type == STMT_RETURN) {
            Data_Type return_type = eval_expr(&fn->scope, &stmt.as._return.value);
            Data_Type_Cmp_Result comparison = compare_data_type(&return_type, &fn->def.return_type);
            if(comparison != DATA_TYPE_CMP_EQUAL) {
                fprintf(stderr, LOC_FMT" Function "SV_FMT" expecting return type of `",
                        LOC_ARGV(stmt.loc), SV_ARGV(fn->def.name));
                dump_data_type(stderr, &fn->def.return_type);
                fprintf(stderr, "` but found `");
                dump_data_type(stderr, &return_type);
                fprintf(stderr, "`\n");
                exit(EXIT_FAILURE);
            }
            compile_expr_into_x86_64_nasm(module, f, &fn->scope, stmt.as._return.value);
            break;
        }
    }
    fprintf(f, "    pop rbp\n");
    fprintf(f, "    ret\n");
}

void compile_into_x86_64_nasm(const char *file_path, Compiled_Module *module)
{
    FILE *f = fopen(file_path, "w");
    if(!f) {
        fatal("Failed to open file file %s", file_path);
    }

    fprintf(f, "section .text\n");
    fprintf(f, "global main\n");
    for(uint32_t i = 0; i < module->functions.count; ++i) {
        compile_func_def_into_x86_64_nasm(module, f, &module->functions.data[i]);
    }
}
