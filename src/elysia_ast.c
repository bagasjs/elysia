#include "elysia_ast.h"
#include "elysia_compiler.h"
#include <string.h>

typedef struct Stmt_Info {
    const char *name;
    Stmt_Type type;
} Stmt_Info;

static const Stmt_Info stmt_infos[COUNT_STMTS] = {
    [STMT_RETURN] = { .name = "Return Statement", .type = STMT_RETURN }, 
    [STMT_VAR_ASSIGN] = { .name = "Variable Assignment", .type = STMT_VAR_ASSIGN }, 
    [STMT_VAR_DEF] = { .name = "Variable Definition", .type = STMT_VAR_DEF }, 
    [STMT_VAR_INIT] = { .name = "Variable Initialization", .type = STMT_VAR_INIT }, 
    [STMT_EXPR] = { .name = "Expression Statement", .type = STMT_EXPR },
    [STMT_IF] = { .name = "If Statement", .type = STMT_IF }, 
    [STMT_WHILE] = { .name = "While Block", .type = STMT_WHILE },
};

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

#define DUMP_PREFIX ' '
#define DUMP(depth, ...) prefix_print(DUMP_PREFIX, depth, __VA_ARGS__)

#include <stdio.h>
void dump_func_def(const Func_Def *func_def, size_t depth)
{
    DUMP(depth, "Function Definition: "SV_FMT"\n", SV_ARGV(func_def->name));
    DUMP(depth + 1, "Return type: ");
    dump_parsed_type(&func_def->return_type);
    putchar('\n');
    DUMP(depth + 1, "Parameters: \n");
    for(size_t i = 0; i < func_def->params.count; ++i) {
        Func_Param param = func_def->params.data[i];
        DUMP(depth + 2, "- "SV_FMT":", SV_ARGV(param.name));
        dump_parsed_type(&param.type);
        putchar('\n');
    }
    DUMP(depth + 1, "Body: \n");
    for(size_t i = 0; i < func_def->body.count; ++i) {
        Stmt stmt = func_def->body.data[i];
        dump_stmt(&stmt, depth + 2);
    }
}

void dump_parsed_type(const Parsed_Data_Type *type)
{
    if(type->is_ptr) {
        putchar('*');
    }
    DUMP(0, SV_FMT, SV_ARGV(type->name));
    if(type->is_array) {
        if(type->array_len != 0) {
            DUMP(0, "[%zu]", type->array_len);
        } else {
            DUMP(0, "[]");
        }
    }
}

void dump_stmt(const Stmt *stmt, size_t depth)
{
    DUMP(depth, "- %s\n", stmt_infos[stmt->type].name);

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
        case STMT_WHILE:
            {
                DUMP(depth + 1, "Do: \n");
                Block block = stmt->as._while.todo;
                for(size_t i = 0; i < block.count; ++i) {
                    Stmt stmt = block.data[i];
                    dump_stmt(&stmt, depth + 2);
                }
            } break;
        default:
            {

            } break;
    }
}

void dump_expr(const Expr *expr, size_t depth)
{
    switch(expr->type) {
        default:
            {
            } break;
    }
}
