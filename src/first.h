#ifndef FIRST_H_
#define FIRST_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#if !defined(ASSERT)
#include <assert.h>
#define ASSERT assert
#endif

#if !defined(MALLOC) && !defined(FREE)
#include <stdlib.h>
#define MALLOC malloc
#define FREE free
#endif

#if !defined(MALLOC) || !defined(FREE)
#error "Please define both MALLOC and FREE macros"
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(xs)(sizeof(xs)/sizeof(*xs))
#endif

/// Dynamic Array

#ifndef DA_INIT_CAPACITY
#define DA_INIT_CAPACITY 32
#endif

#ifndef da
#define da(T) struct { T *items; size_t capacity, count; }
#endif

#ifndef da_free
#define da_free(da) if((da)->items) FREE((da)->items)
#endif

#ifndef da_reset
#define da_reset(da) do { (da)->count = 0; } while(0)
#endif

#ifndef da_append
#define da_append(da, item) \
    do { \
        if((da)->count + 1 > (da)->capacity) { \
            size_t item_size = sizeof(*(da)->items); \
            size_t new_capacity = (da)->capacity * 2; \
            if(new_capacity == 0) new_capacity = DA_INIT_CAPACITY; \
            void *new_items = MALLOC(new_capacity * item_size); \
            ASSERT(new_items != NULL && "Buy More RAM LOL!"); \
            memcpy(new_items, (da)->items, (da)->count*item_size); \
            FREE((da)->items); \
            (da)->items = new_items; \
            (da)->capacity = new_capacity; \
        } \
        (da)->items[(da)->count++] = (item); \
    } while(0);
#endif

#ifndef da_append_many
#define da_append_many(da, new_items, new_items_count) \
    do { \
        size_t item_size = sizeof(*(da)->items); \
        if((da)->count + (new_items_count) > (da)->capacity) { \
            size_t new_capacity = (da)->capacity * 2 + (new_items_count); \
            if(new_capacity == 0) new_capacity = DA_INIT_CAPACITY + (new_items_count); \
            void *old_items = (da)->items; \
            (da)->items = MALLOC(new_capacity * item_size); \
            ASSERT((da)->items != NULL && "Buy More RAM LOL!"); \
            (da)->capacity = new_capacity; \
            if(old_items) { \
                memcpy((da)->items, old_items, (da)->count*item_size); \
                FREE(old_items); \
            } \
        } \
        memcpy((da)->items + (da)->count, (new_items), (new_items_count)*item_size); \
        (da)->count += (new_items_count); \
    } while(0)
#endif

/// String Builder

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} String_Builder;

#ifndef sb_append
#define sb_append(sb, buf, bufsz) da_append_many(sb, buf, bufsz)
#endif

#ifndef sb_append_cstr
#define sb_append_cstr(sb, cstr) \
    do { \
        size_t i; \
        for(i = 0; (cstr)[i] != '\0'; ++i); \
        sb_append(sb, (cstr), i); \
    } while(0)
#endif

/// String View

typedef struct String_View {
    const char *data;
    size_t count;
} String_View;

#define SV_FMT "%.*s"
#define SV_ARGV(sv) (int)sv.count, sv.data
#define SV_DEBUG_FMT "(%zu) %.*s"
#define SV_DEBUG_ARGV(sv) sv.count, (int)sv.count, sv.data
#define SV_STATIC(cstr) { .data = (cstr), .count = sizeof(cstr) }
#define SV(cstr) sv_from_parts(cstr, sizeof(cstr))

String_View sv_from_cstr(const char *cstr);
String_View sv_from_parts(const char *cstr, size_t count);
bool sv_eq(String_View a, String_View b);

#define ARENA_REGION_DEFAULT_CAPACITY (32*1024)

typedef struct Region Region;
typedef struct Arena {
    Region *begin;
    Region *end;
} Arena;
void *arena_alloc(Arena *a, size_t size);
void arena_reset(Arena *a);
void arena_free(Arena *a);

void *temp_alloc(size_t size);
void temp_reset(void);
char *temp_clone_str(const char *str);
char *temp_sprintf(const char *fmt, ...);

typedef struct CMD {
    const char **items;
    size_t capacity;
    size_t count;
} CMD;
#define cmd_append(cmd, ...) \
    da_append_many(cmd, ((const char*[]){__VA_ARGS__}), (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))
void cmd_to_string(CMD cmd, String_Builder *result);
void cmd_reset(CMD *cmd);

int shell_exec(const char *command);
int shell_exec_cmd(CMD cmd);

int  fs_mkdir(const char *path, int permission);
int  fs_rmdir(const char *path);
bool fs_is_exists(const char *path);
bool fs_is_dir(const char *path);
bool fs_is_file(const char *path);
String_View fs_get_filename(String_View path);

#ifndef REBUILD_URSELF
#define REBUILD_URSELF(argc, argv) \
    do { \
        CMD cmd = {0}; \
        const char *prog_name = argv[0]; \
        size_t prog_name_length = strlen(prog_name); \
        char *old_prog_name = MALLOC(strlen(argv[0]) + 4); \
        ASSERT(old_prog_name && "Failed to defer the old build program name"); \
        memcpy(old_prog_name, prog_name, prog_name_length); \
        memcpy(old_prog_name + prog_name_length, ".old", 4); \
        cmd_append(&cmd, "cp", prog_name, old_prog_name); \
        shell_exec_cmd(cmd); \
        da_reset(&cmd); \
        cmd_append(&cmd, CC, "-o", prog_name, __FILE__); \
        shell_exec_cmd(cmd); \
        FREE(old_prog_name); \
        da_free(&cmd); \
    } while(0)
#endif  // REBUILD_URSELF

#endif // FIRST_H_


#ifdef FIRST_IMPLEMENTATION

bool sv_eq(String_View a, String_View b)
{
    if(a.count  != b.count) return false;
    size_t i;
    for(i = 0; i < a.count; ++i) {
        if(a.data[i] != b.data[i])
            return false;
    }
    return true;
}

String_View sv_from_cstr(const char *cstr)
{
    return sv_from_parts(cstr, strlen(cstr));
}

String_View sv_from_parts(const char *cstr, size_t size)
{
    String_View result;
    result.data  = cstr;
    result.count = size;
    return result;
}

void cmd_to_string(CMD cmd, String_Builder *result)
{
    for (size_t i = 0; i < cmd.count; ++i) {
        const char *arg = cmd.items[i];
        if (arg == NULL) break;
        if (i > 0) 
            sb_append_cstr(result, " ");
        if (!strchr(arg, ' ')) {
            sb_append_cstr(result, arg);
        } else {
            da_append(result, '\'');
            sb_append_cstr(result, arg);
            da_append(result, '\'');
        }
    }
    da_append(result, '\0');
}

struct Region {
    Region *next;
    size_t count;
    size_t capacity;
    uintptr_t *data;
};

Region *region_new(size_t capacity)
{
    size_t size_bytes = sizeof(Region) + sizeof(uintptr_t)*capacity;
    Region *r = MALLOC(size_bytes);
    ASSERT(r && "Failed to create new arena region");
    r->next = NULL;
    r->count = 0;
    r->capacity = capacity;
    r->data = (uintptr_t*)(r + 1);
    return r;
}

void region_free(Region *r)
{
    FREE(r);
}

void *arena_alloc(Arena *a, size_t size_bytes)
{
    size_t size = (size_bytes + sizeof(uintptr_t) - 1)/sizeof(uintptr_t);
    if(a->end == NULL) {
        ASSERT(a->begin == NULL);
        size_t capacity = ARENA_REGION_DEFAULT_CAPACITY;
        if(capacity < size) capacity = size;
        a->end = region_new(capacity);
        a->begin = a->end;
    }

    while((a->end->count + size > a->end->capacity) && (a->end->next != NULL)) {
        a->end = a->end->next;
    }

    if(a->end->count + size > a->end->capacity) {
        ASSERT(a->end->next == NULL);
        size_t capacity = ARENA_REGION_DEFAULT_CAPACITY;
        if(capacity < size) capacity = size;
        a->end->next = region_new(capacity);
        a->end = a->end->next;
    }

    void *result = &a->end->data[a->end->count];
    a->end->count += size;
    return result;
}

void arena_reset(Arena *a)
{
    for (Region *r = a->begin; r != NULL; r = r->next) {
        r->count = 0;
    }

    a->end = a->begin;
}

void arena_free(Arena *a)
{
    Region *r = a->begin;
    while (r) {
        Region *r0 = r;
        r = r->next;
        region_free(r0);
    }
    a->begin = NULL;
    a->end = NULL;
}

#ifndef TEMP_CAPACITY
#define TEMP_CAPACITY (1*1024*1024)
#endif

static struct {
    uint8_t data[TEMP_CAPACITY];
    size_t count;
} TEMP;

void *temp_alloc(size_t size)
{
    if(TEMP.count + size > TEMP_CAPACITY) {
        return NULL;
    }
    void *result = (void *)&TEMP.data[TEMP.count];
    TEMP.count += size;
    return result;
}

void temp_reset(void)
{
    TEMP.count = 0;
}

char *temp_clone_str(const char *str)
{
    size_t length = strlen(str);
    char *result = temp_alloc(length + 1);
    if(!result) return NULL;
    memcpy(result, str, length);
    result[length] = '\0';
    return result;
}

char *temp_sprintf(const char *fmt, ...)
{
    va_list ap;
    size_t written;

    va_start(ap, fmt);
    written = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if(written == 0) {
        return NULL;
    }

    char *result = (char *)temp_alloc(written);
    if(!result) {
        return NULL;
    }
    va_start(ap, fmt);
    vsnprintf(result, written, fmt, ap);
    va_end(ap);

    return result;
}

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(__linux__) || defined(__gnu_linux__)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

int shell_exec(const char *command)
{
    return system(command);
}

int shell_exec_cmd(CMD cmd)
{
    String_Builder sb = {0};
    cmd_to_string(cmd, &sb);
    int success = shell_exec(sb.items);
    da_free(&sb);
    return success;
}

int fs_mkdir(const char *path, int permission)
{
    int result = -1;
#ifdef _WIN32
    if(CreateDirectory(path, NULL)) result = 0;
#endif
#if defined(__linux__) || defined(__gnu_linux__)
    result = mkdir(path, permission);
#endif
    return result;
}

int fs_rmdir(const char *path)
{
    int result = - 1;
#ifdef _WIN32
    if(RemoveDirectory(path)) result = 0;
#endif
#if defined(__linux__) || defined(__gnu_linux__)
    result = rmdir(path);
#endif
    return result;
}

bool fs_is_exists(const char *path)
{
    bool result = false;
#ifdef _WIN32
    DWORD attributes = GetFileAttributes(path);
    result = (attributes != INVALID_FILE_ATTRIBUTES);
#endif
#if defined(__linux__) || defined(__gnu_linux__)
    struct stat buffer;
    result = (stat(path, &buffer) == 0);
#endif
    return result;
}

bool fs_is_dir(const char *path)
{
    bool result = false;
#ifdef _WIN32
    DWORD attributes = GetFileAttributes(path);
    result = ((attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY));
#elif defined(__linux__) || defined(__gnu_linux__)
    struct stat buffer;
    if (stat(path, &buffer) == 0) {
        result = S_ISDIR(buffer.st_mode);
    }
#endif
    return result;
}

bool fs_is_file(const char *path)
{
    bool result = false;
#ifdef _WIN32
    DWORD attributes = GetFileAttributes(path);
    result = ((attributes != INVALID_FILE_ATTRIBUTES) && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
#elif defined(__linux__) || defined(__gnu_linux__)
    struct stat buffer;
    if (stat(path, &buffer) == 0) {
        result = S_ISREG(buffer.st_mode);
    }
#endif
    return result;
}

String_View fs_get_filename(String_View path)
{
    size_t pos = path.count;
    while (pos > 0 && path.data[pos - 1] != '/' && path.data[pos - 1] != '\\') {
        pos--;
    }
    String_View result;
    result.data = path.data + pos;
    result.count = path.count - pos;
    return result;
}


#endif // FIRST_IMPLEMENTATION
