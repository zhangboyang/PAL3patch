#include "common.h"

// error handler, must quit, important, don't define to nothing
#define bvec_assert(expr, message) do { \
    if (!(expr)) { \
        fail("bytevector internal error: %s", message); \
    } \
} while (0)


// basic interface

void bvec_ctor(struct bvec *v)
{
    v->begin = v->end = v->capacity = NULL;
}
void bvec_dtor(struct bvec *v)
{
    free(v->begin);
}
void bvec_clear(struct bvec *v)
{
    v->end = v->begin;
}
void bvec_fclear(struct bvec *v)
{
    // clear and free
    bvec_dtor(v);
    bvec_ctor(v);
}
void bvec_copy(struct bvec *dst, const struct bvec *src)
{
    if (dst != src) {
        bvec_fclear(dst);
        bvec_vpush(dst, src);
    }
}
void bvec_move(struct bvec *dst, struct bvec *src)
{
    if (dst != src) {
        bvec_dtor(dst);
        *dst = *src;
        bvec_ctor(src);
    }
}
void bvec_swap(struct bvec *a, struct bvec *b)
{
    struct bvec t;
    t = *a;
    *a = *b;
    *b = t;
}
void *bvec_bdata(const struct bvec *v)
{
    return v->begin;
}
void *bvec_bbegin(const struct bvec *v)
{
    return v->begin;
}
void *bvec_bend(const struct bvec *v)
{
    return v->end;
}
int bvec_empty(const struct bvec *v)
{
    return !bvec_bsize(v);
}
size_t bvec_bsize(const struct bvec *v)
{
    return PTRSUB(v->end, v->begin);
}
size_t bvec_bcapacity(const struct bvec *v)
{
    return PTRSUB(v->capacity, v->begin);
}
static void bvec_brealloc(struct bvec *v, size_t capacity)
{
    // realloc buffer, @capacity should bigger than size
    // internal use only
    size_t size = bvec_bsize(v);
    assert(capacity >= size);
    void *buffer = realloc(v->begin, capacity);
    bvec_assert(buffer || !capacity, "out of memory");
    v->begin = buffer;
    v->end = PTRADD(v->begin, size);
    v->capacity = PTRADD(v->begin, capacity);
}
void bvec_shrink(struct bvec *v)
{
    size_t size = bvec_bsize(v);
    size_t capacity = bvec_bcapacity(v);
    while (capacity && capacity / 2 >= size) capacity /= 2;
    bvec_brealloc(v, capacity);
}
void bvec_breserve(struct bvec *v, size_t size)
{
    // make capacity at least @size bytes, size is unchanged
    // for complexity, double capacity until capacity >= @size
    size_t old_capacity = bvec_bcapacity(v);
    
    size_t new_capacity = old_capacity ? old_capacity : BVEC_DEFAULT_CAPACITY;
    while (new_capacity < size) {
        bvec_assert(new_capacity * 2 > new_capacity, "integer overflow");
        new_capacity *= 2;
    }
    
    if (new_capacity > old_capacity) {
        bvec_brealloc(v, new_capacity);
    }
}
void bvec_bresize(struct bvec *v, size_t size)
{
    // resize vector, if bigger, new space is not initialized
    size_t old_size = bvec_bsize(v);
    if (size > old_size) {
        bvec_breserve(v, size);
    }
    v->end = PTRADD(v->begin, size);
}
void bvec_bpush(struct bvec *v, const void *data, size_t size)
{
    // push (append) bytes
    if (size) {
        size_t old_size = bvec_bsize(v);
        size_t new_size = old_size + size;
        bvec_assert(new_size > old_size, "interger overflow");
        bvec_bresize(v, new_size);
        memcpy(PTRADD(v->begin, old_size), data, size);
    }
}
void bvec_vpush(struct bvec *dst, const struct bvec *src)
{
    assert(dst != src);
    // push (append) another vector
    bvec_bpush(dst, bvec_bdata(src), bvec_bsize(src));
}
void bvec_bpop(struct bvec *v, size_t size)
{
    if (size) {
        size_t old_size = bvec_bsize(v);
        size_t new_size = old_size - size;
        bvec_assert(new_size < old_size, "interger underflow");
        bvec_bresize(v, new_size);
    }
}





// high-level interface (value)

void bvec_push_char(struct bvec *v, char val)
{
    bvec_bpush(v, &val, sizeof(val));
}
void bvec_push_wchar(struct bvec *v, wchar_t val)
{
    bvec_bpush(v, &val, sizeof(val));
}
void bvec_push_int(struct bvec *v, int val)
{
    bvec_bpush(v, &val, sizeof(val));
}
void bvec_push_unsigned(struct bvec *v, unsigned val)
{
    bvec_bpush(v, &val, sizeof(val));
}
void bvec_push_ptr(struct bvec *v, void *val)
{
    bvec_bpush(v, &val, sizeof(val));
}


// high-level interface (strings)

char *bvec_getstr(struct bvec *v)
{
    return bvec_tdata(v, char);
}
void bvec_strshrink(struct bvec *v)
{
    bvec_tresize(v, strlen(bvec_tdata(v, char)) + 1, char);
    bvec_shrink(v);
}
void bvec_strcat(struct bvec *v, const char *str)
{
    if (!bvec_empty(v) && bvec_tback(v, char) == '\0') bvec_tpopback(v, char);
    bvec_tpush(v, str, strlen(str) + 1, char);
}
wchar_t *bvec_getwstr(struct bvec *v)
{
    return bvec_tdata(v, wchar_t);
}
void bvec_wstrshrink(struct bvec *v)
{
    bvec_tresize(v, wcslen(bvec_tdata(v, wchar_t)) + 1, wchar_t);
    bvec_shrink(v);
}
void bvec_wcscat(struct bvec *v, const wchar_t *wstr)
{
    if (!bvec_empty(v) && bvec_tback(v, wchar_t) == L'\0') bvec_tpopback(v, wchar_t);
    bvec_tpush(v, wstr, wcslen(wstr) + 1, wchar_t);
}


// high-level interface (printf)

static struct bvec bvec_vprintf(const char *fmt, va_list ap)
{
    // vprintf to byte vector, internal use only
    
    // alloc buffer
    struct bvec r;
    bvec_ctor(&r);
    bvec_tresize(&r, BVEC_DEFAULT_PRINTF_BUFSIZE, char);

    while (1) {
        char *buf = bvec_tdata(&r, char); 
        size_t bufsize = bvec_tsize(&r, char);
        
        // do snprintf
        va_list aq;
        va_copy(aq, ap);
        vsnprintf(buf, bufsize, fmt, aq);
        va_end(aq);
        
        // work around buggy snprintf implementations
        buf[bufsize - 1] = 0;
        if (strlen(buf) < bufsize - 1) break;
        
        // buffer might not enough, double it and retry
        bvec_assert(bufsize * 2 > bufsize, "integer overflow");
        bvec_tresize(&r, bufsize * 2, char);
    }
    
    bvec_strshrink(&r);
    
    return r;
}
struct bvec bvec_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    struct bvec r = bvec_vprintf(fmt, ap);
    va_end(ap);
    return r;
}
void bvec_strcat_printf(struct bvec *v, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    struct bvec t = bvec_vprintf(fmt, ap);
    va_end(ap);
    bvec_strcat(v, bvec_getstr(&t));
    bvec_dtor(&t);
}

static struct bvec bvec_vwprintf(const wchar_t *fmt, va_list ap)
{
    // vwprintf to byte vector, internal use only
    
    // alloc buffer
    struct bvec r;
    bvec_ctor(&r);
    bvec_tresize(&r, BVEC_DEFAULT_PRINTF_BUFSIZE, wchar_t);

    while (1) {
        wchar_t *buf = bvec_tdata(&r, wchar_t); 
        size_t bufsize = bvec_tsize(&r, wchar_t);
        
        // do snprintf
        va_list aq;
        va_copy(aq, ap);
        vsnwprintf(buf, bufsize, fmt, aq);
        va_end(aq);
        
        // work around buggy snprintf implementations
        buf[bufsize - 1] = 0;
        if (wcslen(buf) < bufsize - 1) break;
        
        // buffer might not enough, double it and retry
        bvec_assert(bufsize * 2 > bufsize, "integer overflow");
        bvec_tresize(&r, bufsize * 2, wchar_t);
    }
    
    bvec_wstrshrink(&r);
    
    return r;
}
struct bvec bvec_wprintf(const wchar_t *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    struct bvec r = bvec_vwprintf(fmt, ap);
    va_end(ap);
    return r;
}
void bvec_wcscat_wprintf(struct bvec *v, const wchar_t *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    struct bvec t = bvec_vwprintf(fmt, ap);
    va_end(ap);
    bvec_wcscat(v, bvec_getwstr(&t));
    bvec_dtor(&t);
}

#if 0
void bvec_selftest()
{
    // NOTE: please set these values to 1 if you want to test realloc
    //   BVEC_DEFAULT_CAPACITY
    //   BVEC_DEFAULT_PRINTF_BUFSIZE

    struct bvec v, v2, v3, v4;
    
    bvec_ctor(&v);
    bvec_ctor(&v2);
    bvec_ctor(&v3);
    bvec_ctor(&v4);

    bvec_strcat(&v, "a");
    bvec_strcat(&v, "bb");
    bvec_strcat_printf(&v, "%c%s%d", 'c', "cc", 12345);
    bvec_strcat(&v, "dddd");
    bvec_strcat(&v, "eeeee");
    assert(bvec_tsize(&v, char) == 16 + 5);
    assert(strcmp(bvec_getstr(&v), "abbccc12345ddddeeeee") == 0);
    bvec_copy(&v2, &v);
    assert(bvec_tsize(&v2, char) == 16 + 5);
    assert(strcmp(bvec_getstr(&v2), "abbccc12345ddddeeeee") == 0);
    bvec_clear(&v);
    assert(bvec_bsize(&v) == 0);
    bvec_wcscat(&v, L"a");
    bvec_wcscat(&v, L"bb");
    bvec_wcscat_wprintf(&v, L"%c%s%d", L'c', L"cc", 12345);
    bvec_wcscat(&v, L"dddd");
    bvec_wcscat(&v, L"eeeee");
    assert(bvec_tsize(&v, wchar_t) == 16 + 5);
    assert(wcscmp(bvec_getwstr(&v), L"abbccc12345ddddeeeee") == 0);
    bvec_copy(&v3, &v);
    assert(bvec_tsize(&v3, wchar_t) == 16 + 5);
    assert(wcscmp(bvec_getwstr(&v3), L"abbccc12345ddddeeeee") == 0);
    bvec_fclear(&v);
    assert(bvec_empty(&v));
    bvec_move(&v4, &v2);
    assert(bvec_empty(&v2));
    assert(bvec_tsize(&v4, char) == 16 + 5);
    assert(strcmp(bvec_getstr(&v4), "abbccc12345ddddeeeee") == 0);
    bvec_swap(&v3, &v4);
    assert(bvec_tsize(&v3, char) == 16 + 5);
    assert(strcmp(bvec_getstr(&v3), "abbccc12345ddddeeeee") == 0);
    assert(bvec_tsize(&v4, wchar_t) == 16 + 5);
    assert(wcscmp(bvec_getwstr(&v4), L"abbccc12345ddddeeeee") == 0);
    bvec_tresize(&v3, 7, char);
    assert(bvec_tsize(&v3, char) == 7);
    assert(strncmp(bvec_getstr(&v3), "abbccc1", 7) == 0);
    bvec_tat(&v3, 3, char) = 0;
    bvec_strshrink(&v3);
    assert(bvec_tsize(&v3, char) == 4);
    assert(strcmp(bvec_getstr(&v3), "abb") == 0);
    bvec_tresize(&v4, 7, wchar_t);
    assert(bvec_tsize(&v4, wchar_t) == 7);
    assert(wcsncmp(bvec_getwstr(&v4), L"abbccc1", 7) == 0);
    bvec_tat(&v4, 3, wchar_t) = 0;
    bvec_wstrshrink(&v4);
    assert(bvec_tsize(&v4, wchar_t) == 4);
    assert(wcscmp(bvec_getwstr(&v4), L"abb") == 0);
    // NOTE: assume little endian, and pointer is 4 byte
    bvec_push_char(&v, '\x11');
    bvec_push_wchar(&v, L'\x2233');
    bvec_push_ptr(&v, NULL);
    bvec_push_int(&v, 0x44556677);
    bvec_push_unsigned(&v, 0x8899AABB);
    assert(bvec_bsize(&v) == 1 + 2 + 4 + 4 + 4);
    assert(memcmp(bvec_bdata(&v), "\x11\x33\x22\x00\x00\x00\x00\x77\x66\x55\x44\xBB\xAA\x99\x88", bvec_bsize(&v)) == 0);
    bvec_clear(&v);
    bvec_strcat(&v, "abcdef");
    bvec_copy(&v2, &v);
    bvec_vpush(&v, &v2);
    assert(bvec_bsize(&v) == (6 + 1) * 2);
    assert(memcmp(bvec_bdata(&v), "abcdef\0abcdef\0", bvec_bsize(&v)) == 0);
    bvec_copy(&v, &v);
    assert(bvec_bsize(&v) == (6 + 1) * 2);
    assert(memcmp(bvec_bdata(&v), "abcdef\0abcdef\0", bvec_bsize(&v)) == 0);
    bvec_move(&v, &v);
    assert(bvec_bsize(&v) == (6 + 1) * 2);
    assert(memcmp(bvec_bdata(&v), "abcdef\0abcdef\0", bvec_bsize(&v)) == 0);
    bvec_swap(&v, &v);
    assert(bvec_bsize(&v) == (6 + 1) * 2);
    assert(memcmp(bvec_bdata(&v), "abcdef\0abcdef\0", bvec_bsize(&v)) == 0);
    
    bvec_dtor(&v);
    bvec_dtor(&v2);
    bvec_dtor(&v3);
    bvec_dtor(&v4);
}
#endif
