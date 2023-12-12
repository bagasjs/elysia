#include "elysia.h"
#include "elysia_ast.h"
#include "elysia_compiler.h"
#include "elysia_lexer.h"
#include "elysia_parser.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void usage(FILE *f)
{
    fprintf(f, "USAGE: elysia SUBCOMMAND <ARGS> [KWARGS]\n");
    fprintf(f, "Available subcommands: \n");
    fprintf(f, "    com <file> <output?> [KWARGS]   Compile program\n");
    fprintf(f, "    tokenize <file>                 Tokenization step\n");
    fprintf(f, "    version                         Get the current compiler version\n");
    fprintf(f, "    help                            Get this message\n");
}

String_View shift(int *argc, char ***argv, const char *error)
{
    if(*argc < 1) {
        usage(stderr);
        fatal("%s", error);
    }
    const char *result = (*argv)[0];
    *argv += 1;
    *argc -= 1;
    return sv_from_parts(result, strlen(result));
}

#define TMPBUF_CAP (32*1024)
static uint8_t tmpbuf[TMPBUF_CAP] = {0};
static size_t tmp_usage = 0;

void tmp_reset()
{
    for(size_t i = 0; i < TMPBUF_CAP; ++i)
        tmpbuf[i] = 0;
    tmp_usage = 0;
}

char *tmp_sprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char *result = (char *)&tmpbuf[tmp_usage];
    tmp_usage += vsnprintf(result, TMPBUF_CAP - tmp_usage - 1, fmt, ap) + 1;
    tmpbuf[tmp_usage - 1] = '\0';
    va_end(ap);
    return result;
}

int main(int argc, char **argv)
{
    shift(&argc, &argv, "Unreachable");

    String_View subcommand = shift(&argc, &argv, "Please provide a subcommand");
    if(sv_eq(subcommand, SV("com"))) {
        Arena arena = {0};
        Lexer lex;
        lex.i = 0;
        String_View source_path = shift(&argc, &argv, "Please provide the source file path");
        const char *source_data = arena_load_file_data(&arena, source_path.data);
        if(!source_data) {
            fatal("Failed to load source file data");
        }

        String_View source = sv_from_parts(source_data, strlen(source_data));
        if(!init_lexer(&lex, source_path, source)) {
            fatal("Failed to initialize the lexer");
        }

        Module mod = parse_module(&arena, &lex);
        dump_func_def(&mod.main, 0);
        compile_into_x86_64_nasm("output.asm", &mod);
    } else {
        fatal("Please provide a valid subcommand");
    }
}
