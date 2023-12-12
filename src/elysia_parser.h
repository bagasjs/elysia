#ifndef ELYSIA_PARSER_H_
#define ELYSIA_PARSER_H_

#include "sv.h"
#include "arena.h"
#include "elysia_lexer.h"
#include "elysia_ast.h"

Data_Type parse_data_type(Arena *arena, Lexer *lex);
Expr parse_expr(Arena *arena, Lexer *lex);
Stmt parse_stmt(Arena *arena, Lexer *lex);
Func_Param_List parse_func_params(Arena *arena, Lexer *lex);
Block parse_block(Arena *arena, Lexer *lex);
Func_Def parse_func_def(Arena *arena, Lexer* lex);
Module parse_module(Arena *arena, Lexer* lex);

#endif // ELYSIA_PARSER_H_
