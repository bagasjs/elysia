#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
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

int main(int argc, char **argv)
{
    shift(&argc, &argv, "Unreachable");

    String_View subcommand = shift(&argc, &argv, "Please provide a subcommand");
    if(sv_eq(subcommand, SV("com"))) {
        Lexer lex;
        lex.i = 0;
        String_View source_path = shift(&argc, &argv, "Please provide the source file path");
        const char *source_data = context_load_file_data(source_path.data);
        if(!source_data) {
            fatal("Failed to load source file data");
        }
        String_View source = sv_from_parts(source_data, strlen(source_data));
        if(!init_lexer(&lex, source)) {
            fatal("Failed to initialize the lexer");
        }

        Module mod = parse_module(&lex);
        printf(SV_FMT"\n", SV_ARGV(mod.main.name));
    } else if(sv_eq(subcommand, SV("tokenize"))) {
        Lexer lex;
        lex.i = 0;
        String_View source_path = shift(&argc, &argv, "Please provide the source file path");
        const char *source_data = context_load_file_data(source_path.data);
        if(!source_data) {
            fatal("Failed to load source file data");
        }
        String_View source = sv_from_parts(source_data, strlen(source_data));
        if(!init_lexer(&lex, source)) {
            fatal("Failed to initialize the lexer");
        }
        Token tok = {0};
        while(next_token(&lex, &tok)) {
            dump_token(tok);
        }
    } else {
        fatal("Please provide a valid subcommand");
    }
}
