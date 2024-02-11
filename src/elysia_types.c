#include "elysia.h"
#include "elysia_ast.h"
#include <stdarg.h>
#include <stdlib.h>

static Native_Type_Info native_type_infos[COUNT_NATIVE_TYPES] = {
    [NATIVE_TYPE_VOID] = { .type = NATIVE_TYPE_VOID, .name = SV_STATIC("void"), .size = 0 },
    [NATIVE_TYPE_BOOL] = { .type = NATIVE_TYPE_BOOL, .name = SV_STATIC("bool"), .size = 1 },
    [NATIVE_TYPE_CHAR] = { .type = NATIVE_TYPE_CHAR, .name = SV_STATIC("char"), .size = 1 },
    [NATIVE_TYPE_U8] = { .type = NATIVE_TYPE_U8, .name = SV_STATIC("u8"), .size = 1 },
    [NATIVE_TYPE_U16] = { .type = NATIVE_TYPE_U16, .name = SV_STATIC("u16"), .size = 2 },
    [NATIVE_TYPE_U32] = { .type = NATIVE_TYPE_U32, .name = SV_STATIC("u32"), .size = 4 },
    [NATIVE_TYPE_U64] = { .type = NATIVE_TYPE_U64, .name = SV_STATIC("u64"), .size = 8 },
    [NATIVE_TYPE_I8] = { .type = NATIVE_TYPE_I8, .name = SV_STATIC("i8"), .size = 1 },
    [NATIVE_TYPE_I16] = { .type = NATIVE_TYPE_I16, .name = SV_STATIC("i16"), .size = 2 },
    [NATIVE_TYPE_I32] = { .type = NATIVE_TYPE_I32, .name = SV_STATIC("i32"), .size = 4 },
    [NATIVE_TYPE_I64] = { .type = NATIVE_TYPE_I64, .name = SV_STATIC("i64"), .size = 8 },
};

Native_Type_Info get_native_type_info(Native_Type type)
{
    return native_type_infos[type];
}

Native_Type_Info *find_native_type_info_by_name(String_View name)
{
    for(int i = 0; i < COUNT_NATIVE_TYPES; ++i) {
        if(sv_eq(name, native_type_infos[i].name)) {
            return &native_type_infos[i];
        }
    }
    return NULL;
}

size_t get_data_type_size(const Data_Type *data_type)
{
    if(data_type->is_ptr) return sizeof(void*);
    if(data_type->is_array) {
        compilation_note(data_type->loc, "Initialization of variable with array data type is not available for now");
        compilation_error(data_type->loc, "Due to unimplemented feature compilation will be terminated");
        compilation_failure();
        return sizeof(void*);
    }

    if(!data_type->is_native) { // struct data type
        const Struct_Info *info = &data_type->as._struct;
        size_t result = 0;
        for(size_t i = 0; i < info->fields.count; ++i) {
            result += get_data_type_size(&info->fields.data[i].type);
        }
        return result;
    }

    Native_Type_Info info = get_native_type_info(data_type->as.native);
    return info.size;
}

Data_Type_Cmp_Result compare_data_type(const Data_Type *a, const Data_Type *b)
{
    if(!sv_eq(a->name, b->name))  {
        return DATA_TYPE_CMP_NOT_EQUAL;
    }

    if(a->is_ptr != b->is_ptr) {
        return DATA_TYPE_CMP_SIGNATURE_ONLY;
    }

    if(a->is_array == b->is_array) {
        if(a->array_len == b->array_len)
            return DATA_TYPE_CMP_EQUAL;
        else
            return DATA_TYPE_CMP_BOTH_ARRAY;
    } else {
        return DATA_TYPE_CMP_BOTH_POINTER;
    }
}

#define DATA_TYPE_FMT "%s"SV_FMT"%s"
#define DATA_TYPE_ARGV(dt) ((dt)->is_ptr ? "*" : ""), SV_ARGV((dt)->name), ((dt)->is_array ? "[]" : "")

void compilation_type_error(Location at, const Data_Type *expectation, const Data_Type *reality, const char *reason, ...)
{
    if(at.row == 0) {
        fprintf(stderr, SV_FMT"Error: ", SV_ARGV(at.file_path));
    } else {
        fprintf(stderr, SV_FMT":%zu:%zu: error: ", SV_ARGV(at.file_path), at.row, at.col);
    }

    fprintf(stderr, "Expecting type "DATA_TYPE_FMT" but found "DATA_TYPE_FMT" due to ",
            DATA_TYPE_ARGV(expectation), DATA_TYPE_ARGV(reality));

    va_list args;
    va_start(args, reason);
    vfprintf(stderr, reason, args);
    va_end(args);
    exit(EXIT_FAILURE);
}
