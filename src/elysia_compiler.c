#include "elysia.h"
#include "elysia_ast.h"
#include "sv.h"
#include "elysia_compiler.h"
#include <stdio.h>


Data_Type eval_expr(const Expr *expr)
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
            } break;
        case EXPR_BINARY_OP:
            {
                Data_Type leftdt = eval_expr(&expr->as.binop->left);
                Data_Type rightdt = eval_expr(&expr->as.binop->right);
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
        default:
            {
                compilation_error(expr->loc, "Unevaluated expression\n");
                compilation_failure();
            } break;
    }
    return result;
}
