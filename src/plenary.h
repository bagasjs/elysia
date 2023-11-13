#ifndef PLENARY_INCLUDED
#define PLENARY_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> // sizeof(), size_t 
#include <string.h> // memcpy(), memset(),
#include <stdbool.h> // bool

// ======================================================
// Platform detection
// ======================================================

// ======================================================
// Configurations
// ======================================================

#define TEMP_CAPACITY (1*1024*1024)
#define DA_INIT_CAPACITY 32
#define REGION_DEFAULT_CAPACITY (8*1024)

// ======================================================
// Macros
// ======================================================

#define CAST(T, v) ((T)(v))
#define SWAP(T, a, b)   \
    do {                \
        T tmp = a;      \
        a = b;          \
        b = tmp;        \
    } while(0)


// configurable macros
#if !defined(ASSERT)
#   include <assert.h>
#   define ASSERT assert
#endif

#if !defined(MALLOC) && !defined(FREE) && !defined(REALLOC)
#   include <stdlib.h>
#   define MALLOC malloc
#   define FREE free
#   define REALLOC realloc
#endif

#if !defined(ASSERT)
#   error "`plenary.h` require the definition of `ASSERT()` macro"
#endif
#if !defined(MALLOC) || !defined(FREE) || !defined(REALLOC)
#   error "`plenary.h` requires you to define `ALLOC()`, `FREE()`, `REALLOC()` macros"
#endif

// string view macros
#define SV_FMT "%.*s"
#define SV_ARGV(sv) (int)sv.count, sv.data
#define SV_DEBUG_FMT "(%zu) \"%.*s\""
#define SV_DEBUG_ARGV(sv) sv.count, (int)sv.count, sv.data
#define SV_STATIC(cstr_lit) { .count = sizeof(cstr_lit) - 1, .data = (cstr_lit) }
#define SV(cstr_lit) sv_from_parts(cstr_lit, sizeof(cstr_lit) - 1)
#define INVALID_SV (String_View){0}

// dynamic array macros
#define da(T) struct { T* data; size_t count, capacity; }
#define da_free(da) FREE((da)->data)
#define da_append(da, item) \
    do {                                                            \
        if((da)->count >= (da)->capacity) {                         \
            size_t new_capacity = (da)->capacity * 2;               \
            if(new_capacity == 0) new_capacity = DA_INIT_CAPACITY;  \
            (da)->data = REALLOC((da)->data,                        \
                    new_capacity * sizeof(*(da)->data));            \
            (da)->capacity = new_capacity;                          \
        }                                                           \
        (da)->data[(da)->count++] = (item);                         \
    } while(0)

#define da_append_many(da, new_items, new_items_count) \
    do {                                                                \
        if((da)->count + new_items_count > (da)->capacity) {            \
            if((da)->capacity == 0) (da)->capacity = DA_INIT_CAPACITY;  \
            (da)->capacity = (da)->capacity * 2 + new_items_count;      \
            (da)->data = REALLOC((da)->data,                            \
                    (da)->capacity * sizeof(*(da)->data));              \
        }                                                               \
        memcpy((da)->data + (da)->count, new_items,                     \
                new_items_count * sizeof(*(da)->data));                 \
        (da)->count += new_items_count;                                 \
    } while(0)


// ======================================================
// Types
// ======================================================

typedef struct {
    const char* data;
    size_t count;
} String_View;

typedef struct Region Region;
struct Region {
    Region* next;
    size_t usage, capacity;
    void* data;
};

typedef struct Arena Arena;
struct Arena {
    Region *first;
    Region *last;
};


// ======================================================
// Functions
// ======================================================


bool __iswhitespace(char ch);
bool __isalpha(char c);
bool __isdigit(char c);
bool __isalnum(char c);

void *temp_alloc(size_t size);
void temp_reset(void);
size_t temp_usage(void);
char *temp_sprintf(const char *fmt, ...);

String_View sv_from_parts(const char* data, size_t n);
String_View sv_slice(String_View strv, size_t start, size_t end);
int sv_find_cstr(String_View strv, const char* sth, size_t index);
int sv_find(String_View strv, String_View sth, size_t index);
bool sv_contains(String_View strv, String_View sth);
bool sv_has_prefix(String_View strv, String_View prefix);
bool sv_has_suffix(String_View strv, String_View suffix);
bool sv_eq(String_View a, String_View b);
String_View sv_ltrim(String_View strv);
String_View sv_rtrim(String_View strv);
String_View sv_take_left_while(String_View strv, bool (*predicate)(char x));
String_View sv_chop_left(String_View* strv, size_t n);
String_View sv_chop_right(String_View* strv, size_t n);
String_View sv_chop_left_while(String_View* strv, bool (*predicate)(char x));
String_View sv_chop_by_delim(String_View* strv, char delim);
String_View sv_chop_by_sv(String_View* strv, String_View sv);
int sv_to_int(String_View strv);

void *arena_alloc(Arena *arena, size_t nbytes);
void *arena_realloc(Arena *arena, void *oldptr, size_t oldsz, size_t newsz);
void arena_reset(Arena *arena);
void arena_free(Arena *arena);

#ifdef __cplusplus
}
#endif
#endif // PLENARY_INCLUDED

#ifdef PLENARY_IMPLEMENTATION

#include <stdint.h> // uint8_t
#include <stdarg.h> // va_list, va_start(), va_end(),
#include <stdio.h>  // vsnprintf()

bool __iswhitespace(char ch)
{
    return ch == '\n' || ch == '\t' || ch == ' ' || ch == '\r';
}

bool __isalpha(char c)
{
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

bool __isdigit(char c)
{
    return ('0' <= c && c <= '9');
}

bool __isalnum(char c)
{
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9');
}

static struct {
    uint8_t data[TEMP_CAPACITY];
    size_t count;
} TEMP;

void *temp_alloc(size_t size)
{
    if(TEMP.count + size > TEMP_CAPACITY) return 0;
    void* result = CAST(void*, &TEMP.data[TEMP.count]);
    return result;
}

void temp_reset(void)
{
    memset(TEMP.data, 0, sizeof(uint8_t)*TEMP.count);
    TEMP.count = 0;
}

size_t temp_usage(void)
{
    return TEMP.count;
}

char *temp_sprintf(const char *fmt, ...)
{
    ASSERT(fmt);
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    ASSERT(n >= 0);
    char* result = temp_alloc(n + 1);
    ASSERT(result != NULL && "Reset the temporary allocator or extend the size of it");
    va_start(args, fmt);
    vsnprintf(result, n + 1, fmt, args);
    va_end(args);
    return result;
}

String_View sv_from_parts(const char* data, size_t n)
{
    return (String_View) {
        .data = data,
        .count = n,
    };
}

String_View sv_slice(String_View strv, size_t start, size_t end)
{
    if(end < start) SWAP(size_t, start, end);

    if(strv.count < start) 
        return INVALID_SV;

    return (String_View) {
        .data = strv.data + start,
        .count = end - start,
    };
}

bool sv_eq(String_View a, String_View b)
{
    if(a.count < b.count)
        return false;
    for(size_t i = 0; i < b.count; ++i) {
        if(a.data[i] != b.data[i]) 
            return false;
    }
    return true;
}

bool sv_contains(String_View strv, String_View sth)
{
    if(strv.count < sth.count)
        return false;
    for(size_t i = 0; i < strv.count; ++i) {
        if(strv.data[i] == sth.data[0]) {
            String_View cmp = sv_slice(strv, i, i + sth.count);
            if(sv_eq(sth, cmp))
                return true;
        }
    }
    return false;
}

bool sv_has_prefix(String_View strv, String_View prefix)
{
    if(strv.count < prefix.count) {
        return false;
    }

    for(size_t i = 0; i < prefix.count; ++i) {
        if(strv.data[i] != prefix.data[i])
            return false;
    }
    return true;
}

bool sv_has_suffix(String_View strv, String_View suffix)
{
    if(strv.count < suffix.count)
        return false;
    for(int i = (int)suffix.count - 1; i >= 0; --i) {
        if(strv.data[strv.count - suffix.count + i] != suffix.data[i])
            return false;
    }
    return true;
}

int sv_find(String_View strv, String_View sth, size_t index)
{
    if(strv.count < sth.count)
        return -1;

    size_t found_count = 0;
    for(size_t i = 0; i < strv.count; ++i) {
        if(strv.data[i] == sth.data[0]) {
            String_View cmp = sv_slice(strv, i, i + sth.count);
            if(sv_eq(sth, cmp)) {
                if(found_count == index)
                    return (int)i;
                ++found_count;
            }
        }
    }

    return -1;
}

String_View sv_ltrim(String_View strv)
{
    size_t i = 0;
    while(__iswhitespace(strv.data[i]))
        i += 1;
    strv.data += i;
    strv.count -= i;
    return strv;
}

String_View sv_rtrim(String_View strv)
{
    if(strv.count == 0) return INVALID_SV;
    size_t i = 0;
    while(__iswhitespace(strv.data[strv.count - i - 1]))
        i += 1;
    strv.count -= i;
    return strv;
}

String_View sv_chop_right(String_View* strv, size_t n)
{
    if(n > strv->count) {
        n = strv->count;
    }

    String_View result = sv_from_parts(strv->data + strv->count - n, n);
    strv->count -= n;
    return result;
}

String_View sv_chop_left(String_View* strv, size_t n)
{
    if(n >= strv->count) {
        n = strv->count;
    }
    String_View result = sv_from_parts(strv->data, n);

    strv->count -= n;
    strv->data += n;
    return result;
}

String_View sv_chop_by_delim(String_View* strv, char delim)
{
    size_t i = 0;
    while(i < strv->count && strv->data[i] != delim) {
        i += 1;
    }

    String_View result = sv_from_parts(strv->data, i);
    if (i < strv->count) {
        strv->count -= i + 1;
        strv->data  += i + 1;
    } else {
        strv->count -= i;
        strv->data  += i;
    }
    return result;
}

String_View sv_chop_by_sv(String_View *sv, String_View thicc_delim)
{
    String_View window = sv_from_parts(sv->data, thicc_delim.count);
    size_t i = 0;
    while (i + thicc_delim.count < sv->count
        && !(sv_eq(window, thicc_delim)))
    {
        i++;
        window.data++;
    }

    String_View result = sv_from_parts(sv->data, i);

    if (i + thicc_delim.count == sv->count) {
        // include last <thicc_delim.count> characters if they aren't
        //  equal to thicc_delim
        result.count += thicc_delim.count;
    }

    // Chop!
    sv->data  += i + thicc_delim.count;
    sv->count -= i + thicc_delim.count;

    return result;
}

String_View sv_chop_left_while(String_View* strv, bool (*predicate)(char x))
{ 
    size_t i = 0;
    while (i < strv->count && predicate(strv->data[i])) {
        i += 1;
    }
    return sv_chop_left(strv, i);
}

String_View sv_take_left_while(String_View strv, bool (*predicate)(char x))
{
    size_t i = 0;
    while (i < strv.count && predicate(strv.data[i])) {
        i += 1;
    }
    return sv_from_parts(strv.data, i);
}

int sv_to_int(String_View strv)
{
    bool is_negative = false;
    if(strv.data[0] == '-') {
        is_negative = true;
        strv.count -= 1;
        strv.data += 1;
    }
    int result = 0;
    for (size_t i = 0; i < strv.count && __isdigit(strv.data[i]); ++i) {
        result = result * 10 + (int) strv.data[i] - '0';
    }
    if(is_negative) result *= -1;
    return result;
}


Region* region_init(size_t capacity)
{
    size_t size = sizeof(Region) + capacity;
    Region* r = (Region*)MALLOC(size);
    ASSERT(r != NULL);
    r->next = NULL;
    r->usage = 0;
    r->capacity = capacity;
    r->data = (void*)(&r->data + sizeof(r->data));
    return r;
}

void region_deinit(Region* r)
{
    FREE(r);
}

void* arena_alloc(Arena* a, size_t size)
{
    if(a->last == NULL) {
        ASSERT(a->first == NULL);
        size_t capacity = REGION_DEFAULT_CAPACITY;
        if(capacity < size) capacity = size;
        a->last = region_init(capacity);
        a->first = a->last;
    }

    while(a->last->usage + size > a->last->capacity && a->last->next != NULL)
    {
        a->last = a->last->next;
    }

    if(a->last->usage + size > a->last->capacity) {
        ASSERT(a->last->next == NULL);
        size_t capacity = REGION_DEFAULT_CAPACITY;
        if(capacity < size) capacity = size;
        a->last = region_init(capacity);
        a->first = a->last;
    }

    void* result = (void*)((size_t)a->last->data + a->last->usage);
    a->last->usage += size;
    return result;
}

void arena_reset(Arena* a)
{
    for(Region* r = a->first; r != NULL; r = r->next) {
        r->usage = 0;
    }
    a->last = a->first;
}

void arena_free(Arena* a)
{
    Region* r = a->first;
    while(r) {
        Region* current = r;
        r = r->next;
        region_deinit(current);
    }
    a->first = NULL;
    a->last = NULL;
}

#endif // PLENARY_IMPLEMENTATION
