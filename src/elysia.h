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

#endif // ELYSIA_H_
