#ifndef LEXER_H_
#define LEXER_H_

#include <stdint.h>
#include "elysia.h"
#include "sv.h"

#define MAXIMUM_LEXER_CACHE_DATA 10

typedef enum {
    TOKEN_UNKNOWN = 0,
    TOKEN_NAME, TOKEN_INTEGER, TOKEN_FLOAT, TOKEN_STRING,
    TOKEN_ADD, TOKEN_SUB, TOKEN_ASTERISK, TOKEN_DIV, TOKEN_MOD,
    TOKEN_ASSIGN, TOKEN_NOT, TOKEN_EQ, TOKEN_NE, TOKEN_GT, TOKEN_GE, TOKEN_LT, TOKEN_LE,
    TOKEN_AND, TOKEN_OR, TOKEN_BAND, TOKEN_BOR, TOKEN_XOR, TOKEN_SHL, TOKEN_SHR,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_COLON, TOKEN_SEMICOLON, TOKEN_LPAREN, TOKEN_RPAREN, 
    TOKEN_LCURLY, TOKEN_RCURLY, TOKEN_LBRACK, TOKEN_RBRACK,

    // Keywords
    TOKEN_FUNCTION, TOKEN_RETURN, TOKEN_VAR, TOKEN_IF, TOKEN_ELSE,
    TOKEN_WHILE, TOKEN_BREAK, TOKEN_CONTINUE,

    TOKEN_AS, TOKEN_STRUCT, TOKEN_TYPE,
} Token_Type;

typedef struct {
    Token_Type type;
    String_View value;
    Location loc;
} Token;

typedef struct {
    size_t i;
    String_View source;
    char cc;
    Location loc;
    struct {
        Token data[MAXIMUM_LEXER_CACHE_DATA];
        size_t head, tail;
        bool carry;
    } cache;
} Lexer;

size_t lexer_cache_count(Lexer *lex);
bool lexer_cache_get(Lexer *lex, size_t i, Token *result);
bool lexer_cache_push(Lexer *lex, Token token);
bool lexer_cache_shift(Lexer *lex, Token *token);

bool init_lexer(Lexer *lex, String_View source_file_path, String_View source);
bool peek_token(Lexer *lex, Token *token, size_t index);
bool next_token(Lexer *lex, Token *token);
bool cache_next_token(Lexer *lex);
void dump_token(Token token);
bool is_token_binops(Token_Type type);
Token expect_token(Lexer *lex, Token_Type type);
Token expect_keyword(Lexer *lex, String_View name);
Token expect_peeked_token(Lexer *lex, Token_Type type, size_t i);
Token expect_peeked_keyword(Lexer *lex, String_View name, size_t i);

#endif // LEXER_H_
