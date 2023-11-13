#include "lexer.h"

typedef struct {
    Token_Type type;
    const char* name;
    const char* hardcode;
    bool is_binary_op_token;
} Token_Info;

static Token_Info _token_info[] = {
    [TOKEN_NAME] = { .type = TOKEN_NAME, .name = "name", .is_binary_op_token = false, },
    [TOKEN_INTEGER] = { .type = TOKEN_INTEGER, .name = "integer literal", .is_binary_op_token = false, },
    [TOKEN_FLOAT] = { .type = TOKEN_FLOAT, .name = "float literal", .is_binary_op_token = false, },
    [TOKEN_STRING] = { .type = TOKEN_STRING, .name = "string literal", .is_binary_op_token = false, },
    [TOKEN_SEMICOLON] = { .type = TOKEN_SEMICOLON, .name = "semicolon", .hardcode = ";", .is_binary_op_token = false },
    [TOKEN_COMMA] = { .type = TOKEN_COMMA, . name = "comma", .hardcode = ",", .is_binary_op_token = false },
    [TOKEN_DOT] = { .type = TOKEN_DOT, . name = "dot", .hardcode = ".", .is_binary_op_token = false },
    [TOKEN_LPAREN] = { .type = TOKEN_LPAREN, . name = "left paren", .hardcode = "(", .is_binary_op_token = false },
    [TOKEN_RPAREN] = { .type = TOKEN_RPAREN, . name = "right paren", .hardcode = ")", .is_binary_op_token = false },
    [TOKEN_LCURLY] = { .type = TOKEN_LCURLY, . name = "left curly", .hardcode = "{", .is_binary_op_token = false },
    [TOKEN_RCURLY] = { .type = TOKEN_RCURLY, . name = "right curly", .hardcode = "}", .is_binary_op_token = false },
    [TOKEN_LBRACK] = { .type = TOKEN_LBRACK, .name = "left bracket", .hardcode = "[", .is_binary_op_token = false },
    [TOKEN_RBRACK] = { .type = TOKEN_RBRACK, .name = "right bracket", .hardcode = "]", .is_binary_op_token = false },
    [TOKEN_ADD] = { .type = TOKEN_ADD, .name = "add", .hardcode = "+", .is_binary_op_token = true },
    [TOKEN_SUB] = { .type = TOKEN_SUB, .name = "sub", .hardcode = "-", .is_binary_op_token = true },
    [TOKEN_MUL] = { .type = TOKEN_MUL, .name = "mul", .hardcode = "*", .is_binary_op_token = true },
    [TOKEN_DIV] = { .type = TOKEN_DIV, .name = "div", .hardcode = "/", .is_binary_op_token = true },
    [TOKEN_MOD] = { .type = TOKEN_MOD, .name = "mod", .hardcode = "%", .is_binary_op_token = true },
    [TOKEN_ASSIGN] = { .type = TOKEN_ASSIGN, .name = "assign", .hardcode = "=", .is_binary_op_token = false },
    [TOKEN_EQ] = { .type = TOKEN_EQ, .name = "eq", .hardcode = "==", .is_binary_op_token = true },
    [TOKEN_NOT] = { .type = TOKEN_NOT, .name = "not", .hardcode = "!", .is_binary_op_token = true },
    [TOKEN_NE] = { .type = TOKEN_NE, .name = "ne", .hardcode = "!=", .is_binary_op_token = true },
    [TOKEN_GT] = { .type = TOKEN_GT, .name = "gt", .hardcode = ">", .is_binary_op_token = true },
    [TOKEN_GE] = { .type = TOKEN_GE, .name = "ge", .hardcode = ">=", .is_binary_op_token = true },
    [TOKEN_LT] = { .type = TOKEN_LT, .name = "lt", .hardcode = "<", .is_binary_op_token = true },
    [TOKEN_LE] = { .type = TOKEN_LE, .name = "le", .hardcode = "<=", .is_binary_op_token = true },
    [TOKEN_AND] = { .type = TOKEN_AND, .name = "and", .hardcode = "&", .is_binary_op_token = true },
    [TOKEN_OR] = { .type = TOKEN_OR, .name = "or", .hardcode = "|", .is_binary_op_token = true },
    [TOKEN_XOR] = { .type = TOKEN_XOR, .name = "xor", .hardcode = "^", .is_binary_op_token = true },
    [TOKEN_SHL] = { .type = TOKEN_SHL, .name = "shl", .hardcode = "<<", .is_binary_op_token = true },
    [TOKEN_SHR] = { .type = TOKEN_SHR, .name = "shr", .hardcode = ">>", .is_binary_op_token = true },
};

bool is_token_binops(Token_Type type)
{
    return _token_info[type].is_binary_op_token;
}

bool advance_lexer(Lexer *lex)
{
    if(lex->i >= lex->source.count) {
        return false;
    }
    lex->i += 1;
    lex->cc = lex->source.data[lex->i];
    lex->loc.col += 1;
    return true;
}

void cache_token(Lexer *lex, Token_Type type, String_View value)
{
    Token *cache = &lex->cache.data[lex->cache.head];
    lex->cache.head = (lex->cache.head + 1) % MAXIMUM_LEXER_CACHE_DATA;
    if(lex->cache.head == lex->cache.tail) {
        fatal("There's too many tokens to be cached");
    }
    cache->type = type;
    cache->value = value;
    cache->loc.col = lex->loc.col - value.count;
    cache->loc.row = lex->loc.row;
}

bool cache_next_token(Lexer *lex)
{
    if(!lex) return false;
    if(lex->i >= lex->source.count) {
        return false;
    }

    while(__iswhitespace(lex->cc)) {
        if(lex->cc == '\n') {
            lex->loc.row += 1;
            lex->loc.col = 0;
        }
        advance_lexer(lex);
    }

    switch(lex->cc) {
        case '.':
            cache_token(lex, TOKEN_DOT, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case ',':
            cache_token(lex, TOKEN_COMMA, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case ';':
            cache_token(lex, TOKEN_SEMICOLON, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '(':
            cache_token(lex, TOKEN_LPAREN, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case ')':
            cache_token(lex, TOKEN_RPAREN, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '{':
            cache_token(lex, TOKEN_LCURLY, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '}':
            cache_token(lex, TOKEN_RCURLY, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '[':
            cache_token(lex, TOKEN_LBRACK, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case ']':
            cache_token(lex, TOKEN_RBRACK, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '+':
            cache_token(lex, TOKEN_ADD, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '-':
            cache_token(lex, TOKEN_SUB, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '*':
            cache_token(lex, TOKEN_MUL, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '/':
            cache_token(lex, TOKEN_DIV, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '%':
            cache_token(lex, TOKEN_MOD, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '&':
            cache_token(lex, TOKEN_AND, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '|':
            cache_token(lex, TOKEN_OR, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '^':
            cache_token(lex, TOKEN_XOR, sv_slice(lex->source, lex->i, lex->i + 1));
            advance_lexer(lex);
            break;
        case '=':
            {
                size_t start = lex->i;
                advance_lexer(lex);
                if(lex->cc == '=') {
                    cache_token(lex, TOKEN_EQ, sv_slice(lex->source, start, start + 2));
                    advance_lexer(lex);
                } else {
                    cache_token(lex, TOKEN_ASSIGN, sv_slice(lex->source, start, start + 1));
                }
            } break;
        case '!':
            {
                size_t start = lex->i;
                advance_lexer(lex);
                if(lex->cc == '=') {
                    cache_token(lex, TOKEN_NE, sv_slice(lex->source, start, start + 2));
                    advance_lexer(lex);
                } else {
                    cache_token(lex, TOKEN_NOT, sv_slice(lex->source, start, start + 1));
                }
            } break;
        case '>':
            {
                size_t start = lex->i;
                advance_lexer(lex);
                if(lex->cc == '=') {
                    cache_token(lex, TOKEN_GE, sv_slice(lex->source, start, start + 2));
                    advance_lexer(lex);
                } else if(lex->cc == '>') {
                    cache_token(lex, TOKEN_SHR, sv_slice(lex->source, start, start + 2));
                    advance_lexer(lex);
                } else {
                    cache_token(lex, TOKEN_GT, sv_slice(lex->source, start, start + 1));
                }
            } break;
        case '<':
            {
                size_t start = lex->i;
                advance_lexer(lex);
                if(lex->cc == '=') {
                    cache_token(lex, TOKEN_LE, sv_slice(lex->source, start, start + 2));
                    advance_lexer(lex);
                } else if(lex->cc == '>') {
                    cache_token(lex, TOKEN_SHL, sv_slice(lex->source, start, start + 2));
                    advance_lexer(lex);
                } else {
                    cache_token(lex, TOKEN_LT, sv_slice(lex->source, start, start + 1));
                }
            } break;
        case '"':
            {
                advance_lexer(lex);
                size_t start = lex->i;
                while(lex->cc != '"') {
                    advance_lexer(lex);
                }
                cache_token(lex, TOKEN_STRING, sv_slice(lex->source, start, lex->i));
                advance_lexer(lex);
            } break;
        default:
            {
                if(__isalpha(lex->cc)) {
                    size_t start = lex->i;
                    while(__isalnum(lex->cc)) {
                        advance_lexer(lex);
                    }
                    cache_token(lex, TOKEN_NAME, sv_slice(lex->source, start, lex->i));
                } else if(__isdigit(lex->cc)) {
                    size_t start = lex->i;
                    bool is_float = false;
                    while(__isdigit(lex->cc) || lex->cc == '.') {
                        if(lex->cc == '.') {
                            if(is_float) {
                                compiler_trap(lex->loc, "Invalid syntax another '.' in a float number literal");
                                exit(EXIT_FAILURE);
                            }
                            is_float = true;
                        }
                        advance_lexer(lex);
                    }
                    cache_token(lex, is_float ? TOKEN_FLOAT : TOKEN_INTEGER, sv_slice(lex->source, start, lex->i));
                } else {
                    cache_token(lex, TOKEN_UNKNOWN, sv_slice(lex->source, lex->i, lex->i + 1));
                    advance_lexer(lex);
                }
            } break;
    }
    return true;
}

bool peek_token(Lexer *lex, Token *token, uint32_t offset)
{
    if(!lex || !token || offset == 0) {
        return false;
    }

    if(lex->i + offset >= lex->source.count) {
        return false;
    }

    for(uint32_t i = offset; i > 0; --i) {
        cache_next_token(lex);
    }

    size_t index = (lex->cache.tail + offset - 1) % MAXIMUM_LEXER_CACHE_DATA;
    *token = lex->cache.data[index];

    return true;
}

bool next_token(Lexer *lex, Token *token)
{
    if(!lex || !token) return false;

    if(lex->cache.head != lex->cache.tail) {
        *token = lex->cache.data[lex->cache.tail];
        lex->cache.tail = (lex->cache.tail + 1) % MAXIMUM_LEXER_CACHE_DATA;
        return true;
    } else if(cache_next_token(lex)) {
        lex->cache.head -= 1;
        *token = lex->cache.data[lex->cache.tail];
        return true;
    } else {
        return false;
    }
}

bool init_lexer(Lexer *lex, String_View source)
{
    if(!lex || source.count == 0) return false;
    lex->i = 0;
    lex->source = source;
    lex->cc = lex->source.data[lex->i];
    lex->cache.head = 0;
    lex->cache.tail = 0;
    return true;
}

void dump_token(Token token)
{
    printf("[%s](%zu,%zu) -> "SV_FMT"\n", _token_info[token.type].name, token.loc.row, token.loc.col, SV_ARGV(token.value));
}

Token expect_token(Lexer *lex, Token_Type type)
{
    Token token = {0};
    if(!next_token(lex, &token)) {
        compiler_trap(lex->loc, "Reached end of file but expecting token `%s`", _token_info[type].name);
    }
    if(token.type != type) {
        compiler_trap(lex->loc, "Expecting token `%s` found `%s` at tokenization level", _token_info[type].name, _token_info[token.type].name);
    }
    return token;
}

Token expect_keyword(Lexer *lex, String_View name)
{
    Token token = expect_token(lex, TOKEN_NAME);
    if(!sv_eq(token.value, name)) {
        compiler_trap(lex->loc, "Expecting `"SV_FMT"` found `"SV_FMT"` at tokenization level", SV_ARGV(name), SV_ARGV(token.value));
    }
    return token;
}
