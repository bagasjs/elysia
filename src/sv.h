#ifndef SV_H_
#define SV_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> // sizeof(), size_t 
#include <stdbool.h> // bool

#ifndef SWAP
#define SWAP(T, a, b)   \
    do {                \
        T tmp = a;      \
        a = b;          \
        b = tmp;        \
    } while(0)
#endif // SWAP

typedef struct {
    const char* data;
    size_t count;
} String_View;

// string view macros
#define SV_FMT "%.*s"
#define SV_ARGV(sv) (int)sv.count, sv.data
#define SV_DEBUG_FMT "(%zu) \"%.*s\""
#define SV_DEBUG_ARGV(sv) sv.count, (int)sv.count, sv.data
#define SV_STATIC(cstr_lit) { .count = sizeof(cstr_lit) - 1, .data = (cstr_lit) }
#define SV(cstr_lit) sv_from_parts(cstr_lit, sizeof(cstr_lit) - 1)
#define INVALID_SV (String_View){0}

bool char_iswhitespace(char ch);
bool char_isalpha(char c);
bool char_isdigit(char c);
bool char_isalnum(char c);

String_View sv_from_cstr(const char *cstr);
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

#ifdef __cplusplus
}
#endif
#endif // SV_H_

#ifdef SV_IMPLEMENTATION

bool char_iswhitespace(char ch)
{
    return ch == '\n' || ch == '\t' || ch == ' ' || ch == '\r';
}

bool char_isalpha(char c)
{
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

bool char_isdigit(char c)
{
    return ('0' <= c && c <= '9');
}

bool char_isalnum(char c)
{
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9');
}

String_View sv_from_cstr(const char *cstr)
{
    size_t i = 0;
    while(cstr[i++] != '\0');
    return (String_View) {
        .data = cstr,
        .count = i,
    };
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
    while(char_iswhitespace(strv.data[i]))
        i += 1;
    strv.data += i;
    strv.count -= i;
    return strv;
}

String_View sv_rtrim(String_View strv)
{
    if(strv.count == 0) return INVALID_SV;
    size_t i = 0;
    while(char_iswhitespace(strv.data[strv.count - i - 1]))
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
    for (size_t i = 0; i < strv.count && char_isdigit(strv.data[i]); ++i) {
        result = result * 10 + (int) strv.data[i] - '0';
    }
    if(is_negative) result *= -1;
    return result;
}

#endif // SV_IMPLEMENTATION
