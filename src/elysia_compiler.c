#include "elysia.h"
#include "sv.h"
#include "elysia_compiler.h"
#include <stdio.h>

Data_Type data_type_from_parsed_type(Parsed_Type type)
{
    Data_Type result;
    result.name = type.name;
    result.is_ptr = type.is_ptr;
    result.is_array = type.is_array;
    result.array_len = type.array_len;
    if(sv_eq(type.name, SV("void"))) {
        if(!type.is_ptr) {
            compilation_error(type.loc, "`void` type is only available to use for pointer type");
        }
        result.size = 8;
    } else if(sv_eq(type.name, SV("i64"))) {
        result.size = 8;
    } else if(sv_eq(type.name, SV("i32"))) {
        result.size = 4;
    } else if(sv_eq(type.name, SV("i16"))) {
        result.size = 2;
    } else if(sv_eq(type.name, SV("i8"))) {
        result.size = 1;
    } else if(sv_eq(type.name, SV("u64"))) {
        result.size = 8;
    } else if(sv_eq(type.name, SV("u32"))) {
        result.size = 4;
    } else if(sv_eq(type.name, SV("u16"))) {
        result.size = 2;
    } else if(sv_eq(type.name, SV("u8"))) {
        result.size = 1;
    } else if(sv_eq(type.name, SV("bool"))) {
        result.size = 1;
    } else if(sv_eq(type.name, SV("char"))) {
        result.size = 1;
    } else {
        compilation_error(type.loc, "Unknown type "SV_FMT"\n", SV_ARGV(type.name));
    }
    return result;
}

void evaluate_module(Module *module)
{
}
