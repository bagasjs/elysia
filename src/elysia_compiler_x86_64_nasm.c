#include "elysia.h"
#include "elysia_ast.h"
#include "elysia_compiler.h"
#include <stdio.h>
#include <stdlib.h>

static void compile_expr_into_x86_64_nasm(FILE *f, const Expr expr)
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
        case EXPR_BINARY_OP:
            {
                compile_expr_into_x86_64_nasm(f, expr.as.binop->right);
                fprintf(f, "    mov rbx, rax\n");
                compile_expr_into_x86_64_nasm(f, expr.as.binop->left);
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
                fatal("Unreachable");
            } break;
    }
}

static void compile_stmt_into_x86_64_nasm(FILE *f, const Stmt stmt)
{
    switch(stmt.type) {
        case STMT_VAR_DEF:
            {

            } break;
        case STMT_VAR_INIT:
            {
            } break;
        case STMT_VAR_ASSIGN:
            {

            } break;
        case STMT_RETURN:
            {
                //
            } break;
        default:
            {
                fatal("Unreachable");
            } break;
    }
}

static void compile_func_def_into_x86_64_nasm(FILE *f, const Module *module, const Func_Def fndef)
{
    fprintf(f, SV_FMT":\n", SV_ARGV(fndef.name));
    fprintf(f, "    push rbp\n");
    fprintf(f, "    mov rbp, rsp\n");
    for(size_t i = 0; i < fndef.body.count; ++i) {
        Stmt stmt = fndef.body.data[i];
        compile_stmt_into_x86_64_nasm(f, stmt);
        if(fndef.body.data[i].type == STMT_RETURN) {
            Data_Type return_type = eval_expr(&stmt.as._return.value);
            Data_Type_Cmp_Result comparison = compare_data_type(&return_type, &fndef.return_type);
            if(comparison != DATA_TYPE_CMP_EQUAL) {
                fprintf(stderr, LOC_FMT" Function "SV_FMT" expecting return type of `",
                        LOC_ARGV(stmt.loc), SV_ARGV(fndef.name));
                dump_data_type(stderr, &fndef.return_type);
                fprintf(stderr, "` but found `");
                dump_data_type(stderr, &fndef.return_type);
                fprintf(stderr, "`\n");
                exit(EXIT_FAILURE);
            }
            compile_expr_into_x86_64_nasm(f, stmt.as._return.value);
            break;
        }
    }
    fprintf(f, "    pop rbp\n");
    fprintf(f, "    ret\n");
}

void compile_into_x86_64_nasm(const char *file_path, const Module *module)
{
    FILE *f = fopen(file_path, "w");
    if(!f) {
        fatal("Failed to open file file %s", file_path);
    }

    fprintf(f, "section .text\n");
    fprintf(f, "global main\n");
    compile_func_def_into_x86_64_nasm(f, module, module->main);
}
