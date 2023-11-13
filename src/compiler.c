#include "compiler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PLENARY_IMPLEMENTATION
#include "plenary.h"

static struct {
    Arena arena;
    String_View source;
} COMPILER;

void _compiler_deinit(int code, void *user_ptr)
{
    arena_free(&COMPILER.arena);
}

bool not_newline(char ch) {
    return ch != '\n' || ch != 0;
}

void compiler_trap(Location loc, const char* fmt, ...)
{
    fprintf(stderr, "Compilation error at line %zu, position %zu:\n -> ", loc.row, loc.col);
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

bool compiler_init(void)
{
    memset(&COMPILER, 0, sizeof(COMPILER));
    on_exit(_compiler_deinit, 0);
    return true;
}

void compiler_set_current_source(String_View source)
{
    COMPILER.source = source;
}


void* context_alloc(size_t nbytes)
{
    return arena_alloc(&COMPILER.arena, nbytes);
}

void context_reset()
{
    arena_reset(&COMPILER.arena);
}

char *context_load_file_data(const char *file_path)
{

    FILE *f = fopen(file_path, "rb");
    if(!f) return NULL;

    size_t file_size = 0;
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *file_data = (char *)context_alloc(file_size + 1);
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
