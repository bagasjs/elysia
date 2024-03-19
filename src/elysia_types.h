#ifndef ELYSIA_TYPES_H_
#define ELYSIA_TYPES_H_

#include "sv.h"
#include "elysia.h"
#include <stdio.h>

typedef enum {
    NATIVE_TYPE_VOID = 0,
    NATIVE_TYPE_U8,
    NATIVE_TYPE_U16,
    NATIVE_TYPE_U32,
    NATIVE_TYPE_U64,
    NATIVE_TYPE_I8,
    NATIVE_TYPE_I16,
    NATIVE_TYPE_I32,
    NATIVE_TYPE_I64,
    NATIVE_TYPE_BOOL,
    NATIVE_TYPE_CHAR,
    COUNT_NATIVE_TYPES,
} Native_Type;

typedef struct {
    Native_Type type;
    String_View name;
    size_t size;
} Native_Type_Info;

typedef struct Data_Type Data_Type;
typedef struct Struct_Field_Info Struct_Field_Info;
typedef enum {
    DATA_TYPE_CMP_NOT_EQUAL = 0,
    DATA_TYPE_CMP_SIGNATURE_ONLY,
    DATA_TYPE_CMP_BOTH_POINTER,
    DATA_TYPE_CMP_BOTH_ARRAY,
    DATA_TYPE_CMP_EQUAL,
} Data_Type_Cmp_Result;

typedef struct {
    String_View name;
    struct {
        Struct_Field_Info *data;
        size_t count;
    } fields;
} Struct_Info;

struct Data_Type {
    Location loc;
    String_View name;
    bool is_native;
    bool is_ptr;
    bool is_array;
    size_t array_len;
    union {
        Native_Type native;
        Struct_Info _struct;
    } as;

    size_t bytesize;
};

struct Struct_Field_Info {
    Data_Type type;
    String_View name;
};

void compilation_type_error(Location at, const Data_Type *expectation, const Data_Type *reality, const char *additional, ...);

Native_Type_Info get_native_type_info(Native_Type type);
Native_Type_Info *find_native_type_info_by_name(String_View name);

void dump_data_type(FILE *f, const Data_Type *type);
void dump_parsed_type(const Data_Type *type);
Data_Type_Cmp_Result compare_data_type(const Data_Type *a, const Data_Type *b);
size_t get_data_type_size(Data_Type *data_type);

#endif // ELYSIA_TYPES_H_
