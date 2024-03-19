#ifndef PLENARY_H_
#define PLENARY_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h> // memcpy, strlen, memset

#define PL_SUCCESS 0
#define PL_ERROR -1 // All error that's not registered will be this
#define PL_INVALID_ARGUMENT -2
#define PL_ALLOCATION_FAILURE -3
#define PL_FILE_DOESNT_EXIST -4

#if !defined(PL_MALLOC) && !defined(PL_FREE)
#include <stdlib.h>
#define PL_MALLOC malloc
#define PL_FREE   free
#endif

#if !defined(PL_MALLOC) || !defined(PL_FREE)
#error "It's look like you're defining either only PL_MALLOC or PL_FREE macros. You should implement both"
#endif

#if !defined(PL_ASSERT)
#include <assert.h>
#define PL_ASSERT assert
#endif

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define _WINUSER_
    #define _WINGDI_
    #define _IMM_
    #define _WINCON_
    #include <windows.h>
    #include <direct.h>
    #include <shellapi.h>
#else
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

////////////////////////////////////////////////////////////////////////
///
/// Dynamic Array
///

#define DA_INIT_CAPACITY 256

#define da(T) struct { T* data; size_t count, capacity; }
#define da_free(da) PL_FREE((da)->data)
#define da_append(da, item) \
    do {                                                                \
        if((da)->count >= (da)->capacity) {                             \
            size_t new_capacity = (da)->capacity * 2;                   \
            if(new_capacity == 0) new_capacity = DA_INIT_CAPACITY;      \
            void *new_data = PL_MALLOC(new_capacity * sizeof(*(da)->data));\
            PL_ASSERT(new_data && "Buy more RAM LOL!");                 \
            memcpy(new_data, (da)->data,                                \
                (da)->count * sizeof(*(da)->data));                     \
            PL_FREE((da)->data);                                        \
            (da)->data = new_data;                                      \
            (da)->capacity = new_capacity;                              \
        }                                                               \
        (da)->data[(da)->count++] = (item);                             \
    } while(0)

#define da_append_many(da, new_items, new_items_count) \
    do {                                                                    \
        if((da)->count + new_items_count > (da)->capacity) {                \
            if((da)->capacity == 0) (da)->capacity = DA_INIT_CAPACITY;      \
            size_t new_capacity = (da)->capacity * 2 + new_items_count;     \
            void *new_data = PL_MALLOC(new_capacity * sizeof(*(da)->data)); \
            PL_ASSERT(new_data && "Buy more RAM LOL!");                     \
            memcpy(new_data, (da)->data, (da)->count * sizeof(*(da)->data));\
            PL_FREE((da)->data);                                            \
            (da)->data = new_data;                                          \
            (da)->capacity = new_capacity;                                  \
        }                                                                   \
        memcpy((da)->data + (da)->count, new_items,                         \
                new_items_count * sizeof(*(da)->data));                     \
        (da)->count += new_items_count;                                     \
    } while(0)

typedef da(char) String_Builder;
#define sb_append(sb, data, size) da_append_many(sb, data, size)
#define sb_append_cstr(sb, cstr)    \
    do {                            \
        const char *s = (cstr);     \
        size_t n = strlen(s);       \
        da_append_many(sb, s, n);   \
    } while (0)
#define sb_append_null(sb) da_append_many(sb, "", 1)
#define sb_free(sb) da_free(sb)

////////////////////////////////////////////////////////////////////////
///
/// Filesystem
///

typedef da(const char *) File_Path_List;

bool fs_exists(const char *file_path);
int fs_mkdir(const char *path, bool recursive);
int fs_rmdir(const char *path);
int fs_listdir(const char *path, File_Path_List *paths);
char *fs_read_file_text(const char *path, size_t *file_size);
uint8_t *fs_read_file_data(const char *path, size_t *file_size);

////////////////////////////////////////////////////////////////////////
///
/// Shell command
///

#ifdef _WIN32
typedef HANDLE Proc;
#define INVALID_PROC INVALID_HANDLE_VALUE
#else
typedef int Proc;
#define INVALID_PROC (-1)
#endif

typedef da(Proc) Proc_List;
bool proc_wait(Proc procs);
bool proc_list_wait(Proc_List proc);

typedef struct {
    const char **data;
    size_t count;
    size_t capacity;
} CMD;

#define cmd_append(cmd, ...) \
    da_append_many(cmd, ((const char*[]){__VA_ARGS__}), (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))

Proc cmd_exec_async(CMD cmd);
bool cmd_exec(CMD cmd);
void cmd_to_string(CMD cmd, String_Builder *render);

////////////////////////////////////////////////////////////////////////
///
/// Temporary memory allocator
///
#ifndef PL_TEMP_CAPACITY
#define PL_TEMP_CAPACITY (8*1024*1024)
#endif

void *pl_temp_alloc(size_t size);
void  pl_temp_reset(void);
void  pl_temp_sprintf(const char *fmt, ...);
void  pl_temp_sprintf_v(const char *fmt, va_list ap);

typedef enum {
    LOG_DEFAULT = 0,
    LOG_ERROR = 1,
    LOG_WARN  = 2,
    LOG_INFO  = 3,
} Log_Level;

#ifndef PLENARY_NO_LOGGING
#include <stdio.h>
void pl_tracelog(Log_Level level, const char *fmt, ...);
#define TRACELOG(level, ...) pl_tracelog(level, __VA_ARGS__)
#else
#define TRACELOG(level, ...) 
#endif

#endif // PLENARY_H_

#ifdef PLENARY_IMPLEMENTATION

#endif // PLENARY_IMPLEMENTATION
