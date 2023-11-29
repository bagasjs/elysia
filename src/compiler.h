#ifndef ELYSIA_COMPILER_H_
#define ELYSIA_COMPILER_H_

#include "plenary.h"

#define ELYSIA_MESSAGE_STACK_CAPACITY 32

typedef struct {
    size_t row, col; 
} Location;

typedef enum {
    MESSAGE_INFO = 0,
    MESSAGE_WARNING,
    MESSAGE_ERROR,
} Message_Level;

void fatal(const char *fmt, ...);
void push_message(Message_Level level, const char *fmt, ...);

void compiler_set_current_source(String_View source);
void compiler_trap(Location at, const char* fmt, ...);

void* context_alloc(size_t nbytes);
void  context_reset();
char *context_load_file_data(const char *file_path);

#endif // ELYSIA_COMPILER_H_
