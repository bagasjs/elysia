#include "elysia.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define SV_IMPLEMENTATION
#include "sv.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"

void compilation_note(Location loc, const char *fmt, ...)
{
    if(loc.row == 0) {
        fprintf(stderr, SV_FMT": note: ", SV_ARGV(loc.file_path));
    } else {
        fprintf(stderr, SV_FMT":%zu:%zu: note: ", SV_ARGV(loc.file_path), loc.row, loc.col);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void compilation_warning(Location loc, const char *fmt, ...)
{
    if(loc.row == 0) {
        fprintf(stderr, SV_FMT": warning: ", SV_ARGV(loc.file_path));
    } else {
        fprintf(stderr, SV_FMT":%zu:%zu: warning: ", SV_ARGV(loc.file_path), loc.row, loc.col);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void compilation_error(Location loc, const char *fmt, ...)
{
    if(loc.row == 0) {
        fprintf(stderr, SV_FMT": error: ", SV_ARGV(loc.file_path));
    } else {
        fprintf(stderr, SV_FMT":%zu:%zu: error: ", SV_ARGV(loc.file_path), loc.row, loc.col);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void fatal(const char *fmt, ...)
{
    fprintf(stderr, "ERROR: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

void prefix_print(char prefix, size_t prefix_count, const char *fmt, ...)
{
    for(size_t i = 0; i < prefix_count; ++i) {
        putchar(prefix);
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

char *arena_load_file_data(Arena *arena, const char *file_path)
{
    FILE *f = fopen(file_path, "rb");
    if(!f) return NULL;

    size_t file_size = 0;
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *file_data = (char *)arena_alloc(arena, file_size + 1);
    if(!file_data) {
        fclose(f);
        return NULL;
    }

    size_t bytes_read = fread(file_data, 1, file_size, f);
    if (bytes_read != file_size) {
        fclose(f);
        free(file_data);
        return NULL;
    }

    file_data[file_size] = 0;
    file_data[file_size - 1] = 0;
    fclose(f);
    return file_data;
}

static Native_Type_Info native_type_infos[COUNT_NATIVE_TYPES] = {
    [NATIVE_TYPE_VOID] = { .type = NATIVE_TYPE_VOID, .name = SV_STATIC("void"), .size = 0 },
    [NATIVE_TYPE_BOOL] = { .type = NATIVE_TYPE_BOOL, .name = SV_STATIC("bool"), .size = 0 },
    [NATIVE_TYPE_CHAR] = { .type = NATIVE_TYPE_CHAR, .name = SV_STATIC("char"), .size = 0 },
    [NATIVE_TYPE_U8] = { .type = NATIVE_TYPE_U8, .name = SV_STATIC("u8"), .size = 0 },
    [NATIVE_TYPE_U16] = { .type = NATIVE_TYPE_U16, .name = SV_STATIC("u16"), .size = 0 },
    [NATIVE_TYPE_U32] = { .type = NATIVE_TYPE_U32, .name = SV_STATIC("u32"), .size = 0 },
    [NATIVE_TYPE_U64] = { .type = NATIVE_TYPE_U64, .name = SV_STATIC("u64"), .size = 0 },
    [NATIVE_TYPE_I8] = { .type = NATIVE_TYPE_I8, .name = SV_STATIC("i8"), .size = 0 },
    [NATIVE_TYPE_I16] = { .type = NATIVE_TYPE_I16, .name = SV_STATIC("i16"), .size = 0 },
    [NATIVE_TYPE_I32] = { .type = NATIVE_TYPE_I32, .name = SV_STATIC("i32"), .size = 0 },
    [NATIVE_TYPE_I64] = { .type = NATIVE_TYPE_I64, .name = SV_STATIC("i64"), .size = 0 },
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

void compilation_failure(void)
{
    fprintf(stderr, "Compilation is terminated due to failure.");
    exit(EXIT_FAILURE);
}
