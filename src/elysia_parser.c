#include "elysia_parser.h"
#include "elysia_compiler.h"
#include "elysia_lexer.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

String_View FUNCTION_KEYWORD = SV_STATIC("fun");
String_View VOID_KEYWORD = SV_STATIC("void");
String_View TRUE_KEYWORD = SV_STATIC("true");
String_View FALSE_KEYWORD = SV_STATIC("false");
String_View RETURN_KEYWORD = SV_STATIC("return");
String_View IF_KEYWORD = SV_STATIC("if");
String_View ELSE_KEYWORD = SV_STATIC("else");
String_View WHILE_KEYWORD = SV_STATIC("while");
String_View BREAK_KEYWORD = SV_STATIC("break");
String_View VAR_KEYWORD = SV_STATIC("var");
String_View CONST_KEYWORD = SV_STATIC("const");

Module parse_module(Arena *arena, Lexer *lex)
{
    Module module = {0};
    Token token = {0};
    while(peek_token(lex, &token, 0)) {
        if(token.type != TOKEN_NAME) {
            compilation_error(token.loc, "Expecting token `name` but got `"SV_FMT"`", SV_ARGV(token.value));
        }

        if(sv_eq(token.value, FUNCTION_KEYWORD)) {
            module.main = parse_func_def(arena, lex);
        } else {
            compilation_error(token.loc, "Expecting keyword `%s` for function definition found `"SV_FMT"`", FUNCTION_KEYWORD, SV_ARGV(token.value));
        }
    }

    return module;
}

Func_Def parse_func_def(Arena *arena, Lexer *lex)
{
    Func_Def result = {0};
    result.loc = expect_keyword(lex, FUNCTION_KEYWORD).loc;
    result.name = expect_token(lex, TOKEN_NAME).value;
    result.params = parse_func_params(arena, lex);

    Token token = {0};
    if(!peek_token(lex, &token, 0)) {
        compilation_error(token.loc, "Expecting function body or return type of the function but found end of file");
    }

    if(token.type == TOKEN_NAME) {
        result.return_type_name = expect_token(lex, TOKEN_NAME).value;
    } else {
        result.return_type_name = VOID_KEYWORD;
    }

    result.body = parse_block(arena, lex);
    return result;
}


Parsed_Data_Type parse_data_type(Arena *arena, Lexer *lex)
{
    expect_token(lex, TOKEN_COLON);
    Parsed_Data_Type result = {0};
    Token token;
    if(peek_token(lex, &token, 0) && token.type == TOKEN_ASTERISK) {
        expect_token(lex, TOKEN_ASTERISK);
        result.is_ptr = true;
    }

    token = expect_token(lex, TOKEN_NAME);
    result.name = token.value;
    if(peek_token(lex, &token, 0) && token.type == TOKEN_LBRACK) {
        expect_token(lex, TOKEN_LBRACK);
        result.is_array = true;
        if(peek_token(lex, &token, 0) && token.type == TOKEN_INTEGER) {
            result.array_len = sv_to_int(token.value);
        }
        expect_token(lex, TOKEN_RBRACK);
    }

    return result;
}

Func_Param_List parse_func_params(Arena *arena, Lexer *lex)
{
    Func_Param_List params = {0};
    expect_token(lex, TOKEN_LPAREN);

    Token token;
    if(peek_token(lex, &token, 0) && token.type == TOKEN_RPAREN) {
        next_token(lex, &token);
        return params;
    } else {
        Func_Param param = {0};
        token = expect_token(lex, TOKEN_NAME);
        param.loc = token.loc;
        param.name = token.value;
        param.type = parse_data_type(arena, lex);
        push_param_to_param_list(arena, &params, param);
    }

    if(peek_token(lex, &token, 0) && token.type == TOKEN_RPAREN) {
        next_token(lex, &token);
        return params;
    }

    while(peek_token(lex, &token, 0) && token.type == TOKEN_COMMA) {
        next_token(lex, &token);

        Func_Param param = {0};
        token = expect_token(lex, TOKEN_NAME);
        param.loc = token.loc;
        param.name = token.value;
        param.type = parse_data_type(arena, lex);
        push_param_to_param_list(arena, &params, param);
    }

    expect_token(lex, TOKEN_RPAREN);
    return params;
}

Block parse_block(Arena *arena, Lexer *lex)
{
    Block result = {0};
    expect_token(lex, TOKEN_LCURLY);

    Token token = {0};
    if(!peek_token(lex, &token, 0)) {
        compilation_error(lex->loc, "Expecting a block but reached end of file\n");
    }

    while(token.type != TOKEN_RCURLY) {
        Stmt stmt = parse_stmt(arena, lex);
        push_stmt_to_block(arena, &result, stmt);
        if(!peek_token(lex, &token, 0)) {
            compilation_error(lex->loc, "Expecting a block but reached end of file\n");
        }
    }
    expect_token(lex, TOKEN_RCURLY);
    return result;
}


Stmt parse_stmt(Arena *arena, Lexer *lex)
{
    Token token = {0};
    if(!peek_token(lex, &token, 0)) {
        compilation_error(lex->loc, "Expecting statement but reached end of file\n");
    }

    Stmt result = {0};

    switch(token.type) {
        case TOKEN_NAME:
            {
                if(sv_eq(token.value, RETURN_KEYWORD)) {
                    token = expect_keyword(lex, RETURN_KEYWORD);
                    result.loc = token.loc;
                    result.type = STMT_RETURN;
                    result.as._return.loc = token.loc;
                    result.as._return.value = parse_expr(arena, lex);
                } else if(sv_eq(token.value, IF_KEYWORD)) {
                    expect_keyword(lex, IF_KEYWORD);
                    result.loc = token.loc;
                    result.type = STMT_IF;
                    assert(0 && "Not implemented");
                } else if(sv_eq(token.value, WHILE_KEYWORD)) {
                    expect_keyword(lex, WHILE_KEYWORD);
                    result.loc = token.loc;
                    result.type = STMT_WHILE;
                    assert(0 && "Not implemented");
                } else if(sv_eq(token.value, VAR_KEYWORD)) {
                    expect_keyword(lex, VAR_KEYWORD);
                    String_View name = expect_token(lex, TOKEN_NAME).value;
                    Token token0 = {0};
                    if(!peek_token(lex, &token0, 0)) {
                        compilation_error(lex->loc, "Expecting something after variable name but found nothing");
                    }

                    result.loc = token.loc;
                    Parsed_Data_Type data_type = {0};
                    bool has_data_type = false;

                    if(token0.type == TOKEN_COLON) {
                        has_data_type = true;
                        data_type = parse_data_type(arena, lex);
                    }

                    // everything related to data type should have been parsed
                    if(!next_token(lex, &token0)) {
                        compilation_error(lex->loc, "Expecting something after variable name but found nothing");
                    }

                    if(token0.type == TOKEN_ASSIGN) {
                        result.type = STMT_VAR_INIT;
                        result.as.var_init.name = name;
                        if(has_data_type) {
                            result.as.var_init.type = data_type;
                        }
                        result.as.var_init.value = parse_expr(arena, lex);
                    } else if(has_data_type) {
                        result.type = STMT_VAR_DEF;
                        result.as.var_def.name = name;
                        result.as.var_def.type = data_type;
                    } else {
                        compilation_error(lex->loc, "Expecting defined variable to have any kind of type anotation");
                    }
                } else {
                }
            } break;
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_LPAREN:
        case TOKEN_RPAREN:
            {
                result.type = STMT_EXPR;
                result.loc = token.loc;
                result.as.expr = parse_expr(arena, lex);
            } break;

        default:
            {
                fatal(SV_FMT"\n", SV_ARGV(token.value));
                assert(0 && "Unreachable");
            } break;
    }

    if(peek_token(lex, &token, 0) && token.type == TOKEN_SEMICOLON) {
        expect_token(lex, TOKEN_SEMICOLON);
    }

    return result;
}

Expr_List parse_func_args(Arena *arena, Lexer *lex)
{
    Expr_List list = {0};
    expect_token(lex, TOKEN_LPAREN);

    Token token = {0};
    while(peek_token(lex, &token, 0) && token.type != TOKEN_RPAREN) {
        push_expr_to_expr_list(arena, &list, parse_expr(arena, lex));
        expect_token(lex, TOKEN_COMMA);
    }
    expect_token(lex, TOKEN_RPAREN);
    return list;
}

Expr parse_expr(Arena *arena, Lexer *lex)
{
    Token token = {0};
    if(!peek_token(lex, &token, 0)) {
        compilation_error(lex->loc, "Expecting expression but got end of file\n");
    }

    Expr result = {0};
    switch(token.type) {
        case TOKEN_NAME:
            {
                if(sv_eq(token.value, TRUE_KEYWORD)) {
                    token = expect_keyword(lex, TRUE_KEYWORD);
                    result.loc = token.loc;
                    result.type = EXPR_BOOL_LITERAL;
                    result.as.literal_bool = true;
                } else if(sv_eq(token.value, FALSE_KEYWORD)) {
                    token = expect_keyword(lex, FALSE_KEYWORD);
                    next_token(lex, &token);
                    result.loc = token.loc;
                    result.as.literal_bool = false;
                    result.type = EXPR_BOOL_LITERAL;
                } else {
                    token = expect_token(lex, TOKEN_NAME);
                    Expr var_read = {0};
                    var_read.as.var_read.name = token.value;
                    var_read.as.var_read.loc = token.loc;

                    Token ntoken = {0};
                    peek_token(lex, &ntoken, 0);
                    if(ntoken.type == TOKEN_LPAREN) {
                        result.type = EXPR_FUNCALL;
                        result.loc = token.loc;
                        result.as.func_call.loc = ntoken.loc;
                        result.as.func_call.args = parse_func_args(arena, lex);
                    } else if(binary_op_type_from_token_type(ntoken.type) != BINARY_OP_UNKNOWN) {
                        next_token(lex, &ntoken);
                        result.type = EXPR_BINARY_OP;
                        result.as.binop = arena_alloc(arena, sizeof(struct Expr_Binary_Op));
                        if(!result.as.binop) {
                            fatal("Failed to allocate for expression: Buy more RAM LOL");
                        }
                        result.loc = ntoken.loc;
                        result.as.binop->type = binary_op_type_from_token_type(ntoken.type);
                        result.as.binop->loc = ntoken.loc;
                        result.as.binop->left = var_read;
                        result.as.binop->right = parse_expr(arena, lex);
                    } else {
                        result = var_read;
                    }
                }
            } break;

        case TOKEN_INTEGER:
            {
                token = expect_token(lex, TOKEN_INTEGER);
                Expr left = {0};
                left.loc = token.loc;
                left.type = EXPR_INTEGER_LITERAL;
                left.as.literal_int = sv_to_int(token.value);

                Token ntoken = {0};
                peek_token(lex, &ntoken, 0);
                Binary_Op_Type op_type = binary_op_type_from_token_type(ntoken.type);

                if(op_type != BINARY_OP_UNKNOWN) {
                    token = expect_token(lex, ntoken.type);
                    result.loc = token.loc;
                    result.type = EXPR_BINARY_OP;
                    result.as.binop = arena_alloc(arena, sizeof(struct Expr_Binary_Op));
                    if(!result.as.binop) {
                        fatal("Failed to allocate for binary op expression: Buy more RAM LOL");
                    }
                    result.as.binop->loc = ntoken.loc;
                    result.as.binop->type = op_type;
                    result.as.binop->left = left;
                    result.as.binop->right = parse_expr(arena, lex);
                } else {
                    result = left;
                }

            } break;
        case TOKEN_STRING:
            {
                next_token(lex, &token);
                result.loc = token.loc;
                result.type = EXPR_INTEGER_LITERAL;
                result.as.literal_str = token.value;
            } break;
        default:
            {
                fatal("Token: "SV_FMT"\n", SV_ARGV(token.value));
                assert(0 && "Unreachable: THIS IS IN DEVELOPMENT MODE");
            } break;
    }

    return result;
}

Binary_Op_Type binary_op_type_from_token_type(Token_Type type)
{
    switch(type)
    {
        case TOKEN_ADD:  return BINARY_OP_ADD;
        case TOKEN_SUB:  return BINARY_OP_SUB;
        case TOKEN_ASTERISK:  return BINARY_OP_MUL;
        case TOKEN_DIV:  return BINARY_OP_DIV;
        case TOKEN_MOD:  return BINARY_OP_MOD;
        case TOKEN_EQ:   return BINARY_OP_EQ;
        case TOKEN_NE:   return BINARY_OP_NE;
        case TOKEN_GT:   return BINARY_OP_GT;
        case TOKEN_GE:   return BINARY_OP_GE;
        case TOKEN_LT:   return BINARY_OP_LT;
        case TOKEN_LE:   return BINARY_OP_LE;
        case TOKEN_AND:  return BINARY_OP_AND;
        case TOKEN_OR:   return BINARY_OP_OR;
        case TOKEN_XOR:  return BINARY_OP_XOR;
        case TOKEN_SHL:  return BINARY_OP_SHL;
        case TOKEN_SHR:  return BINARY_OP_SHR;
        default: return BINARY_OP_UNKNOWN;
    }
}

void push_param_to_param_list(Arena *arena, Func_Param_List *params, Func_Param param)
{
    if(params->count >= params->capacity) {
        size_t new_capacity = params->capacity * 2;
        if(new_capacity == 0) new_capacity = 32;
        void *new_data = arena_alloc(arena, new_capacity * sizeof(*params->data));
        assert(new_data && "Buy more RAM LOL!");
        memcpy(new_data, params->data, params->count * sizeof(*params->data));
        params->data = new_data;
        params->capacity = new_capacity;
    }

    params->data[params->count++] = param;
}

void push_expr_to_expr_list(Arena *arena, Expr_List *list, Expr expr)
{
    if(list->count >= list->capacity) {
        size_t new_capacity = list->capacity * 2;
        if(new_capacity == 0) new_capacity = 32;
        void *new_data = arena_alloc(arena, new_capacity * sizeof(*list->data));
        assert(new_data && "buy more ram lol!");
        memcpy(new_data, list->data, list->count * sizeof(*list->data));
        list->data = new_data;
        list->capacity = new_capacity;
    }

    list->data[list->count++] = expr;
}

void push_stmt_to_block(Arena *arena, Block *block, Stmt stmt)
{
    if(block->count >= block->capacity) {
        size_t new_capacity = block->capacity * 2;
        if(new_capacity == 0) new_capacity = 32;
        void *new_data = arena_alloc(arena, new_capacity * sizeof(*block->data));
        assert(new_data && "buy more ram lol!");
        memcpy(new_data, block->data, block->count * sizeof(*block->data));
        block->data = new_data;
        block->capacity = new_capacity;
    }

    block->data[block->count++] = stmt;
}


