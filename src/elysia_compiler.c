#include "elysia.h"
#include "elysia_ast.h"
#include "elysia_types.h"
#include "sv.h"
#include "elysia_compiler.h"
#include <stdio.h>

const Evaluated_Var *get_var_from_scope(const Scope *scope, String_View name)
{
    for(size_t i = 0; i < scope->vars.count; ++i) {
        const Evaluated_Var *var = &scope->vars.data[i];
        if(sv_eq(name, var->name)) {
            return var;
        }
    }
    return NULL;
}

bool emplace_var_to_scope(Scope *scope, String_View name, Data_Type type, size_t address)
{
    if(scope->vars.count + 1 > ELYSIA_SCOPE_VARS_CAPACITY) {
        return false;
    }

    scope->vars.data[scope->vars.count].name = name;
    scope->vars.data[scope->vars.count].type = type;
    scope->vars.data[scope->vars.count].address = address;
    scope->vars.count += 1;
    return true;
}

bool push_fn_to_module(Evaluated_Module *module, const Evaluated_Fn fn)
{
    if(module->functions.count + 1 > ELYSIA_MODULE_FUNCTIONS_CAPACITY) {
        return false;
    }
    module->functions.data[module->functions.count++] = fn;
    return true;
}

bool emplace_fn_to_module(Evaluated_Module *module, const Func_Def def)
{
    if(module->functions.count + 1 > ELYSIA_MODULE_FUNCTIONS_CAPACITY) {
        return false;
    }

    module->functions.data[module->functions.count].def = def;
    module->functions.data[module->functions.count].scope.parent = &module->global;
    module->functions.data[module->functions.count].scope.vars.count = 0;
    module->functions.count += 1;
    return true;
}

void eval_stmt(Evaluated_Module *module, Evaluated_Fn *fn, Scope *scope, const Stmt stmt)
{
    switch(stmt.type) {
        case STMT_VAR_DEF:
            {
                Data_Type data_type = stmt.as.var_def.type;
                emplace_var_to_scope(scope, stmt.as.var_def.name, data_type, scope->stack_usage);
                scope->stack_usage += get_data_type_size(&data_type);
            } break;
        case STMT_VAR_INIT:
            {
                size_t addr = scope->stack_usage;
                Data_Type variable_type = eval_expr(module, scope, &stmt.as.var_init.value);
                if(!stmt.as.var_init.infer_type) {
                    if(compare_data_type(&variable_type, &stmt.as.var_init.type) != DATA_TYPE_CMP_EQUAL) {
                        compilation_type_error(stmt.loc, &variable_type, &stmt.as.var_init.type, 
                                "while assigning value to variable `"SV_FMT"`", SV_ARGV(stmt.as.var_init.name));
                    }
                }
                size_t variable_size = 0;
                variable_size = get_data_type_size(&variable_type);
                scope->stack_usage += variable_size;
                // printf("Variable "SV_FMT" with %s type "SV_FMT" with size %zu\n", 
                //         SV_ARGV(stmt.as.var_init.name), variable_type.is_native ? "native" : "non-native", 
                //         SV_ARGV(variable_type.name), variable_size);
                emplace_var_to_scope(scope, stmt.as.var_init.name, variable_type, addr);
            } break;
        case STMT_VAR_ASSIGN:
            {
                const Evaluated_Var *var = get_var_from_scope(scope, stmt.as.var_assign.name);
                Data_Type variable_type = eval_expr(module, scope, &stmt.as.var_assign.value);
                if(compare_data_type(&variable_type, &stmt.as.var_init.type) != DATA_TYPE_CMP_EQUAL) {
                    compilation_type_error(stmt.loc, &variable_type, &var->type, " while assigning value to variable "SV_FMT, 
                            SV_ARGV(stmt.as.var_assign.name));
                }
            } break;
        case STMT_WHILE:
            {

            } break;
        case STMT_RETURN:
            {

                Data_Type return_type = eval_expr(module, &fn->scope, &stmt.as._return.value);
                Data_Type_Cmp_Result comparison = compare_data_type(&return_type, &fn->def.return_type);
                if(comparison != DATA_TYPE_CMP_EQUAL) {
                    compilation_type_error(stmt.loc, &fn->def.return_type, &return_type, 
                            "for the return value of function `"SV_FMT"`", SV_ARGV(fn->def.name));
                }
                fn->has_return_stmt = true;
            } break;
        default:
            {
                fatal("Unreachable");
            } break;
    }
}

void eval_func_def(Evaluated_Module *module, const Func_Def fdef)
{
    Evaluated_Fn result;
    result.has_return_stmt = false;
    result.def = fdef;
    result.scope.vars.count = 0;
    result.scope.stack_usage = 0;
    result.scope.parent = &module->global;
    for(size_t i = 0; i < fdef.body.count; ++i) 
        eval_stmt(module, &result, &result.scope, fdef.body.data[i]);
    if(!result.has_return_stmt) {
        if(!(fdef.return_type.is_native && fdef.return_type.as.native == NATIVE_TYPE_VOID)) {
            compilation_error(fdef.loc, "Function `"SV_FMT"` doesn't have any return statement but it's not a void function",
                    SV_ARGV(fdef.name));
        }
    }
    push_fn_to_module(module, result);
}

Data_Type eval_expr(Evaluated_Module *module, const Scope *scope, const Expr *expr)
{
    Data_Type result;
    switch(expr->type) {
        case EXPR_INTEGER_LITERAL:
            {
                Native_Type_Info info = get_native_type_info(NATIVE_TYPE_I32);
                result.name = info.name;
                result.loc = expr->loc;
                result.is_ptr = false;
                result.is_array = false;
                result.array_len = 0;
                result.is_native = true;
                result.as.native = NATIVE_TYPE_I32;
                result.bytesize = 0;
            } break;
        case EXPR_BINARY_OP:
            {
                Data_Type leftdt = eval_expr(module, scope, &expr->as.binop->left);
                // Data_Type rightdt = eval_expr(module, scope, &expr->as.binop->right);
                switch(expr->as.binop->type) {
                    case BINARY_OP_ADD:
                        {
                            if(leftdt.is_ptr) {
                                compilation_error(expr->loc, 
                                        "Left operand of binary operation where the type is a pointer is "
                                        "not allowed\n");
                                compilation_failure();
                            }
                        } break;
                    default:
                        {
                            compilation_error(expr->loc, "Failed to evaluate expression's result data type\n");
                            compilation_failure();
                        } break;
                result = leftdt;
                }
            } break;
        case EXPR_VAR_READ:
            {
                String_View var_name = expr->as.var_read.name;
                const Evaluated_Var *var = get_var_from_scope(scope, var_name);
                if(var == NULL) {
                    compilation_error(expr->loc, "Failed to read into unknown variable\n");
                }
                result = var->type;
            } break;
        default:
            {
                compilation_error(expr->loc, "Unevaluated expression\n");
                compilation_failure();
            } break;
    }
    return result;
}

bool eval_module(Evaluated_Module *result, const Module *module)
{
    for(size_t i = 0; i < module->functions.count; ++i) 
        eval_func_def(result, module->functions.data[i]);
    return true;
}
