#include "elysia.h"
#include "elysia_ast.h"
#include "elysia_compiler.h"
#include <stdio.h>

static void compile_function_x86_64_nasm(FILE *f, const Func_Def *def);
static void compile_stmt_x86_64_nasm(FILE *f, const Stmt *stmt);
static void compile_expr_x86_64_nasm(FILE *f, const Expr *expr);

void compile_x86_64_nasm(const char *file_path, const Module *module)
{
    FILE *f = fopen(file_path, "w");
    if(!f) {
        fatal("Failed to open file file %s", file_path);
    }

    fprintf(f, "section .text\n");
    fprintf(f, "global _start\n");
    compile_function_x86_64_nasm(f, &module->main);
    fprintf(f, "_start:\n");
    fprintf(f, "    call main\n");
    fprintf(f, "    mov rax, 60\n");
    fprintf(f, "    mov rdi, 0\n");
    fprintf(f, "    syscall\n");
    fprintf(f, "    ret\n");
}

void compile_function_x86_64_nasm(FILE *f, const Func_Def *def)
{
    fprintf(f, SV_FMT":\n", SV_ARGV(def->name));
    fprintf(f, "    push rbp\n");
    fprintf(f, "    mov  rbp, rsp\n");
    Block body = def->body;
    for(size_t i = 0; i < body.count; ++i) {
        compile_stmt_x86_64_nasm(f, &body.data[i]);
    }
    fprintf(f, "    pop  rbp\n");
    fprintf(f, "    ret");
}

void compile_stmt_x86_64_nasm(FILE *f, const Stmt *stmt)
{
    switch(stmt->type) {
        case STMT_VAR_DEF:
            {
            } break;
        case STMT_VAR_INIT:
            {
            } break;
        case STMT_VAR_ASSIGN:
            {
            } break;
        default:
            {
            } break;
    }
}

void compile_expr_x86_64_nasm(FILE *f, const Expr *expr)
{
    switch(expr->type) {
        default:
            {
            } break;
    }
}
