#ifndef ELYSIA_H_
#define ELYSIA_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "sv.h"
#include "arena.h"

#define ELYSIA_MESSAGE_STACK_CAPACITY 32

typedef struct {
    String_View file_path;
    size_t row, col;
} Location;

void fatal(const char *fmt, ...);

void compilation_note(Location at, const char *fmt, ...);
void compilation_warning(Location at, const char *fmt, ...);
void compilation_error(Location at, const char *fmt, ...);
char *arena_load_file_data(Arena *arena, const char *file_path);
void prefix_print(char prefix, size_t prefix_count, const char *fmt, ...);


#endif // ELYSIA_H_
