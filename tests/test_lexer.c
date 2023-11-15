#include "compiler.h"
#include "lexer.h"
#include <math.h>

const char *source_data = 
    "fun main() {"
    "   let name = \"Hello, World\\n\";"
    "   let pi   = 3.14;"
    "   let test = 1 + 2 - 3 * 4 / 5;"
    "}"
    ;

int main(void)
{
    Lexer lex;
    lex.i = 0;
    String_View source = sv_from_parts(source_data, strlen(source_data));
    if(!init_lexer(&lex, source)) {
        fatal("Failed to initialize the lexer\n");
    }

    printf("PEEKING TOKEN\n");
    Token tok = {0};
    if(!peek_token(&lex, &tok, 0)) {
        fprintf(stderr, "Failed to peek token cached_size=%zu\n", lexer_cache_count(&lex));
        return 1;
    }
    dump_token(tok);

    printf("LOADING ALL TOKEN\n");
    while(next_token(&lex, &tok)) {
        dump_token(tok);
    }
    return 0;
}
