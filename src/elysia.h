#ifndef ELYSIA_H_
#define ELYSIA_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "sv.h"
#include "arena.h"

typedef struct {
    String_View file_path;
    size_t row, col;
} Location;

#define LOC_FMT  SV_FMT":%zu:%zu:"
#define LOC_ARGV(loc) SV_ARGV(loc.file_path), loc.row, loc.col

void fatal(const char *fmt, ...);

void compilation_note(Location at, const char *fmt, ...);
void compilation_warning(Location at, const char *fmt, ...);
void compilation_error(Location at, const char *fmt, ...);
void compilation_failure(void);
char *arena_load_file_data(Arena *arena, const char *file_path);
void prefix_print(char prefix, size_t prefix_count, const char *fmt, ...);

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

Native_Type_Info get_native_type_info(Native_Type type);
Native_Type_Info *find_native_type_info_by_name(String_View name);

#endif // ELYSIA_H_
