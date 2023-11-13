#ifndef ELYSIA_COMPILER_H_
#define ELYSIA_COMPILER_H_

#include "plenary.h"

typedef struct { size_t row, col; } Location;
void fatal(const char *fmt, ...);

bool compiler_init(void);
void compiler_trap(Location at, const char* fmt, ...);
void compiler_set_current_source(String_View source);

void* context_alloc(size_t nbytes);
void  context_reset();

char *context_load_file_data(const char *file_path);

#endif // ELYSIA_COMPILER_H_
