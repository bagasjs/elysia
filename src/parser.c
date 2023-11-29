#include "parser.h"
#include "compiler.h"
#include "plenary.h"
#include "lexer.h"
#include <assert.h>

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

Module parse_module(Lexer *lex)
{
    Module module = {0};
    Token token = {0};
    while(peek_token(lex, &token, 0)) {
        if(token.type != TOKEN_NAME) {
            compiler_trap(token.loc, "Expecting token `name` but got `"SV_FMT"`", SV_ARGV(token.value));
        }

        if(sv_eq(token.value, FUNCTION_KEYWORD)) {
            module.main = parse_func_def(lex);
        } else {
            compiler_trap(token.loc, "Expecting keyword `%s` for function definition found `"SV_FMT"`", FUNCTION_KEYWORD, SV_ARGV(token.value));
        }
    }

    return module;
}

Func_Def parse_func_def(Lexer *lex)
{
    Func_Def result = {0};
    result.loc = expect_keyword(lex, FUNCTION_KEYWORD).loc;
    result.name = expect_token(lex, TOKEN_NAME).value;
    result.params = parse_func_params(lex);

    Token token = {0};
    if(!peek_token(lex, &token, 0)) {
        compiler_trap(token.loc, "Expecting function body or return type of the function but found end of file");
    }

    if(token.type == TOKEN_NAME) {
        result.return_type_name = expect_token(lex, TOKEN_NAME).value;
    } else {
        result.return_type_name = VOID_KEYWORD;
    }

    result.body = parse_block(lex);
    return result;
}

Func_Param_List parse_func_params(Lexer *lex)
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
        param.type_name = expect_token(lex, TOKEN_NAME).value;
        da_append(&params, param);
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
        param.type_name = expect_token(lex, TOKEN_NAME).value;
        da_append(&params, param);
    }

    expect_token(lex, TOKEN_RPAREN);
    return params;
}

Block parse_block(Lexer *lex)
{
    Block result = {0};
    expect_token(lex, TOKEN_LCURLY);

    Token token = {0};
    if(!peek_token(lex, &token, 0)) {
        compiler_trap(lex->loc, "Expecting a block but reached end of file\n");
    }

    while(token.type != TOKEN_RCURLY) {
        Stmt stmt = parse_stmt(lex);
        da_append(&result, stmt);
        if(!peek_token(lex, &token, 0)) {
            compiler_trap(lex->loc, "Expecting a block but reached end of file\n");
        }
    }
    expect_token(lex, TOKEN_RCURLY);
    return result;
}


Stmt parse_stmt(Lexer *lex)
{
    Token token = {0};
    if(!peek_token(lex, &token, 0)) {
        compiler_trap(lex->loc, "Expecting statement but reached end of file\n");
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
                    result.as._return.value = parse_expr(lex);
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
                    if(!next_token(lex, &token0)) {
                        compiler_trap(lex->loc, "Expecting something after variable name but found nothing");
                    }

                    if(token0.type == TOKEN_NAME) {
                        // var x int;  // definition
                    } else if(token0.type == TOKEN_ASSIGN) {
                        // var x = 10; // initialization
                    } else {
                        compiler_trap(token.loc, "Expecting this line to be either variable defintion or variable initialization");
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
                result.as.expr = parse_expr(lex);
            } break;

        default:
            {
                fprintf(stderr, SV_FMT"\n", SV_ARGV(token.value));
                assert(0 && "Unreachable");
            } break;
    }

    if(peek_token(lex, &token, 0) && token.type == TOKEN_SEMICOLON) {
        expect_token(lex, TOKEN_SEMICOLON);
    }

    return result;
}

Expr_List parse_func_args(Lexer *lex)
{
    Expr_List list = {0};
    expect_token(lex, TOKEN_LPAREN);

    Token token = {0};
    while(peek_token(lex, &token, 0) && token.type != TOKEN_RPAREN) {
        da_append(&list, parse_expr(lex));
        expect_token(lex, TOKEN_COMMA);
    }
    expect_token(lex, TOKEN_RPAREN);
    return list;
}

Expr parse_expr(Lexer *lex)
{
    Token token = {0};
    if(!peek_token(lex, &token, 0)) {
        compiler_trap(lex->loc, "Expecting expression but got end of file\n");
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
                    result.type = EXPR_BOOL_LITERAL;
                    result.as.literal_bool = false;
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
                        result.as.func_call.params = parse_func_args(lex);
                    } else if(binary_op_type_from_token_type(ntoken.type) != BINARY_OP_UNKNOWN) {
                        next_token(lex, &ntoken);
                        result.type = EXPR_BINARY_OP;
                        result.as.binop = context_alloc(sizeof(struct Expr_Binary_Op));
                        if(!result.as.binop) {
                            fatal("Failed to allocate for expression: Buy more RAM LOL");
                        }
                        result.loc = ntoken.loc;
                        result.as.binop->type = binary_op_type_from_token_type(ntoken.type);
                        result.as.binop->loc = ntoken.loc;
                        result.as.binop->left = var_read;
                        result.as.binop->right = parse_expr(lex);
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
                    result.as.binop = context_alloc(sizeof(struct Expr_Binary_Op));
                    if(!result.as.binop) {
                        fatal("Failed to allocate for binary op expression: Buy more RAM LOL");
                    }
                    result.as.binop->loc = ntoken.loc;
                    result.as.binop->type = op_type;
                    result.as.binop->left = left;
                    result.as.binop->right = parse_expr(lex);
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
                fprintf(stderr, "Token: "SV_FMT"\n", SV_ARGV(token.value));
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
        case TOKEN_MUL:  return BINARY_OP_MUL;
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
