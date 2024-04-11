#include "elysia_parser.h"

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

typedef struct Expr_Info {
    const char *name;
    Expr_Type type;
} Expr_Info;

static const Expr_Info expr_infos[COUNT_EXPRS] = {
    [EXPR_INTEGER_LITERAL] = { .name = "Integer Literal", .type = EXPR_INTEGER_LITERAL, }, 
    [EXPR_STRING_LITERAL] = { .name = "String Literal", .type = EXPR_STRING_LITERAL, }, 
    [EXPR_BOOL_LITERAL] = { .name = "Bool Literal", .type = EXPR_BOOL_LITERAL, }, 
    [EXPR_FLOAT_LITERAL] = { .name = "Float Literal", .type = EXPR_FLOAT_LITERAL, },
    [EXPR_FUNCALL] = { .name = "Function Call", .type = EXPR_FUNCALL, }, 
    [EXPR_VAR_READ] = { .name = "Variable Read", .type = EXPR_VAR_READ, }, 
    [EXPR_BINARY_OP] = { .name = "Binary Operation", .type = EXPR_BINARY_OP, },
};

#define DUMP_PREFIX ' '
#define DUMP(depth, ...) prefix_print(DUMP_PREFIX, depth, __VA_ARGS__)
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

void dump_data_type(FILE *f, const Data_Type *type)
{
    if(type->is_ptr) {
        putchar('*');
    }

    fprintf(f, SV_FMT, SV_ARGV(type->name));
    if(type->is_array) {
        if(type->array_len != 0) {
            fprintf(f, "[%zu]", type->array_len);
        } else {
            fprintf(f, "[]");
        }
    }
}

void dump_parsed_type(const Data_Type *type)
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
    if(type->is_native) {
        printf(" (native)");
    }
}

void dump_stmt(const Stmt *stmt, size_t depth)
{
    DUMP(depth, "- %s\n", stmt_infos[stmt->type].name);

    switch(stmt->type) {
        case STMT_VAR_DEF:
            {
                DUMP(depth + 1, "Name: "SV_FMT"\n", SV_ARGV(stmt->as.var_def.name));
                DUMP(depth + 1, "Type:");
                dump_parsed_type(&stmt->as.var_def.type);
                putchar('\n');
            } break;
        case STMT_VAR_INIT:
            {
                DUMP(depth + 1, "Name: "SV_FMT"\n", SV_ARGV(stmt->as.var_init.name));
                DUMP(depth + 1, "Type:");
                dump_parsed_type(&stmt->as.var_init.type);
                putchar('\n');
                DUMP(depth + 1, "Value:\n");
                dump_expr(&stmt->as.var_init.value, depth + 2);
            } break;
        case STMT_VAR_ASSIGN:
            {
                DUMP(depth + 1, "Name: "SV_FMT"\n", SV_ARGV(stmt->as.var_assign.name));
                dump_expr(&stmt->as.var_assign.value, depth + 2);
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
    DUMP(depth, "- %s\n", expr_infos[expr->type].name);
}

