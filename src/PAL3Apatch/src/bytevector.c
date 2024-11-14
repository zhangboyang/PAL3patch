#include "common.h"

// error handler, must quit, important, don't define to nothing
#define bvec_assert(expr, message) do { \
    if (!(expr)) { \
        fail("bytevector internal error: %s", message); \
    } \
} while (0)

// debug asserts, can be removed safely
#ifdef BVEC_DEBUG
#define bvec_dbgassert(x) assert(x)
#else
#define bvec_dbgassert(x)
#endif

// debug fillbyte
#ifdef BVEC_DEBUG
#define BVEC_FILLBYTE 0xCD
void *bvec_dbgmemset(void *s, int c, size_t n)
{
    // safe version of memset, to avoid undefined behavior is s == NULL
    //   if s == NULL && n == 0, then do nothing
    bvec_dbgassert(s || !n);
    if (s) memset(s, c, n);
    return s;
}
#endif

// basic interface

void bvec_ctor(struct bvec *v)
{
    v->begin = v->end = v->capacity = NULL;
}
void bvec_bctor(struct bvec *v, const void *data, size_t size)
{
    bvec_ctor(v);
    bvec_bpush(v, data, size);
}
void bvec_cctor(struct bvec *v, const struct bvec *src)
{
    bvec_ctor(v);
    bvec_cpush(v, src);
}
void bvec_dtor(struct bvec *v)
{
    free(v->begin);
#ifdef BVEC_DEBUG
    memset(v, BVEC_FILLBYTE, sizeof(struct bvec));
#endif
}
void *bvec_mdtor(struct bvec *v)
{
    return v->begin;
}
void bvec_clear(struct bvec *v)
{
    v->end = v->begin;
#ifdef BVEC_DEBUG
    bvec_dbgmemset(v->begin, BVEC_FILLBYTE, PTRSUB(v->capacity, v->begin));
#endif
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
        bvec_dtor(dst);
        bvec_cctor(dst, src);
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
    if (a != b) {
        struct bvec t;
        t = *a;
        *a = *b;
        *b = t;
    }
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
    // if @capacity == 0, then free previous data
    // internal use only
    size_t size = bvec_bsize(v);
    bvec_dbgassert(capacity >= size);
    void *buffer = realloc(v->begin, capacity);
    bvec_assert(buffer || !capacity, "out of memory");
#ifdef BVEC_DEBUG
    bvec_dbgmemset(PTRADD(buffer, size), BVEC_FILLBYTE, capacity - size);
#endif
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
void bvec_cpush(struct bvec *dst, const struct bvec *src)
{
    bvec_dbgassert(dst != src);
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




// string template

#define BVEC_STRING_IMPL(clsname, tchar, tname, printf) \
\
void CONCAT3(clsname, _, ctor)(struct clsname *s) \
{ \
    tchar t = 0; \
    bvec_tctor(&s->v, &t, 1, tchar); \
} \
void CONCAT3(clsname, _, cctor)(struct clsname *s, const struct clsname *src) \
{ \
    bvec_cctor(&s->v, &src->v); \
} \
void CONCAT3(clsname, _, sctor)(struct clsname *s, const tchar *str) \
{ \
    bvec_tctor(&s->v, str, CONCAT(tname, len)(str) + 1, tchar); \
} \
void CONCAT3(clsname, _, dtor)(struct clsname *s) \
{ \
    bvec_dtor(&s->v); \
} \
tchar *CONCAT3(clsname, _, mdtor)(struct clsname *s) \
{ \
    return bvec_mdtor(&s->v); \
} \
void CONCAT3(clsname, _, clear)(struct clsname *s) \
{ \
    tchar t = 0; \
    bvec_clear(&s->v); \
    bvec_tpushback(&s->v, &t, tchar); \
} \
void CONCAT3(clsname, _, fclear)(struct clsname *s) \
{ \
    CONCAT3(clsname, _, dtor)(s); \
    CONCAT3(clsname, _, ctor)(s); \
} \
void CONCAT3(clsname, _, copy)(struct clsname *dst, const struct clsname *src) \
{ \
    if (dst != src) { \
        bvec_copy(&dst->v, &src->v); \
    } \
} \
void CONCAT3(clsname, _, move)(struct clsname *dst, struct clsname *src) \
{ \
    if (dst != src) { \
        tchar t = 0; \
        bvec_move(&dst->v, &src->v); \
        bvec_tpushback(&src->v, &t, tchar); \
    } \
} \
void CONCAT3(clsname, _, swap)(struct clsname *a, struct clsname *b) \
{ \
    if (a != b) { \
        bvec_swap(&a->v, &b->v); \
    } \
} \
const tchar *CONCAT4(clsname, _, get, tname)(const struct clsname *s) \
{ \
    return bvec_tdata(&s->v, tchar); \
} \
tchar *CONCAT3(clsname, _, data)(const struct clsname *s) \
{ \
    return bvec_tdata(&s->v, tchar); \
} \
tchar *CONCAT3(clsname, _, begin)(const struct clsname *s) \
{ \
    return bvec_tbegin(&s->v, tchar); \
} \
tchar *CONCAT3(clsname, _, end)(const struct clsname *s) \
{ \
    return bvec_tend(&s->v, tchar) - 1; \
} \
int CONCAT3(clsname, _, empty)(const struct clsname *s) \
{ \
    return !CONCAT3(clsname, _, size)(s); \
} \
size_t CONCAT3(clsname, _, size)(const struct clsname *s) \
{ \
    return bvec_tsize(&s->v, tchar) - 1; \
} \
void CONCAT3(clsname, _, shrink)(struct clsname *s) \
{ \
    bvec_shrink(&s->v); \
} \
size_t CONCAT4(clsname, _, tname, len)(const struct clsname *s) \
{ \
    return CONCAT3(clsname, _, size)(s); \
} \
void CONCAT4(clsname, _, tname, cpy)(struct clsname *s, const tchar *str) \
{ \
    bvec_clear(&s->v); \
    bvec_tpush(&s->v, str, CONCAT(tname, len)(str) + 1, tchar); \
} \
void CONCAT4(clsname, _, tname, cat)(struct clsname *s, const tchar *str) \
{ \
    bvec_tpopback(&s->v, tchar); \
    bvec_tpush(&s->v, str, CONCAT(tname, len)(str) + 1, tchar); \
} \
void CONCAT4(clsname, _, tname, ncat)(struct clsname *s, const tchar *str, size_t n) \
{ \
    if (n) { \
        size_t l; \
        for (l = 0; l < n && str[l]; l++); \
        CONCAT3(clsname, _, push)(s, str, l); \
    } \
} \
void CONCAT3(clsname, _, push)(struct clsname *s, const tchar *str, size_t n) \
{ \
    if (n) { \
        tchar t = 0; \
        bvec_tpopback(&s->v, tchar); \
        bvec_tpush(&s->v, str, n, tchar); \
        bvec_tpushback(&s->v, &t, tchar); \
    } \
} \
void CONCAT3(clsname, _, pop)(struct clsname *s, size_t n) \
{ \
    if (n) { \
        size_t old_size = CONCAT3(clsname, _, size)(s); \
        size_t new_size = old_size - n; \
        bvec_assert(new_size < old_size, "interger underflow"); \
        CONCAT3(clsname, _, trunc)(s, new_size); \
    } \
} \
void CONCAT3(clsname, _, cpush)(struct clsname *dst, const struct clsname *src) \
{ \
    CONCAT3(clsname, _, push)(dst, CONCAT3(clsname, _, data)(src), CONCAT3(clsname, _, size)(src)); \
} \
void CONCAT3(clsname, _, trunc)(struct clsname *s, size_t n) \
{ \
    tchar t = 0; \
    bvec_tresize(&s->v, n, tchar); \
    bvec_tpushback(&s->v, &t, tchar); \
} \
void CONCAT3(clsname, _, pushback)(struct clsname *s, tchar c) \
{ \
    tchar t = 0; \
    *CONCAT3(clsname, _, end)(s) = c; \
    bvec_tpushback(&s->v, &t, tchar); \
} \
void CONCAT3(clsname, _, popback)(struct clsname *s) \
{ \
    CONCAT3(clsname, _, end)(s)[-1] = 0; \
    bvec_tpopback(&s->v, tchar); \
} \
tchar *CONCAT3(clsname, _, getbuffer)(struct clsname *s, size_t size) \
{ \
    /* get a buffer of @size tchars       */ \
    /*   including NULL tchar             */ \
    /* if @size == 0, size is not changed */ \
    /*   and content is preserved         */ \
    /* it's only vaild to call:           */ \
    /*   commitbuffer()                   */ \
    /*   discardbuffer()                  */ \
    /*   dtor()                           */ \
    /* after calling this function        */ \
     \
    if (size) bvec_tresize(&s->v, size, tchar); \
    return bvec_tdata(&s->v, tchar); \
} \
void CONCAT3(clsname, _, commitbuffer)(struct clsname *s) \
{ \
    bvec_tresize(&s->v, CONCAT(tname, len)(CONCAT3(clsname, _, data)(s)) + 1, tchar); \
} \
void CONCAT3(clsname, _, discardbuffer)(struct clsname *s) \
{ \
    CONCAT3(clsname, _, clear)(s); \
} \
int CONCAT3(clsname, _, vformat)(struct clsname *s, const tchar *fmt, va_list ap) \
{ \
    int ret; \
    size_t size = BVEC_STRING_DEFAULT_FORMATBUFFER; \
     \
    while (1) { \
        tchar *buf = CONCAT3(clsname, _, getbuffer)(s, size); \
        va_list aq; \
        va_copy(aq, ap); \
        ret = CONCAT(vsn, printf)(buf, size, fmt, aq); \
        va_end(aq); \
         \
        buf[size - 1] = 0; \
        if (CONCAT(tname, len)(buf) < size - 1) break; \
         \
        bvec_assert(size * 2 > size, "integer overflow"); \
        CONCAT3(clsname, _, discardbuffer)(s); \
        size = size * 2; \
    } \
    CONCAT3(clsname, _, commitbuffer)(s); \
     \
    return ret; \
} \
int CONCAT3(clsname, _, format)(struct clsname *s, const tchar *fmt, ...) \
{ \
    int ret; \
    va_list ap; \
    va_start(ap, fmt); \
    ret = CONCAT3(clsname, _, vformat)(s, fmt, ap); \
    va_end(ap); \
    return ret; \
} \
// TEMPLATE END

BVEC_STRING_IMPL(wstr, wchar_t, wcs, wprintf)
BVEC_STRING_IMPL(cstr, char, str, printf)


// wstr wrapper


void wstr_cs2wcs(struct wstr *s, const char *cstr, UINT src_cp)
{
    wchar_t *t = cs2wcs_alloc(cstr, src_cp);
    wstr_wcscpy(s, t);
    free(t);
}
void cstr_wcs2cs(struct cstr *s, const wchar_t *wstr, UINT dst_cp)
{
    char *t = wcs2cs_alloc(wstr, dst_cp);
    cstr_strcpy(s, t);
    free(t);
}
void cstr_cs2cs(struct cstr *s, const char *cstr, UINT src_cp, UINT dst_cp)
{
    char *t = cs2cs_alloc(cstr, src_cp, dst_cp);
    cstr_strcpy(s, t);
    free(t);
} 



// unit test

#if 0
void bvec_cstr_selftest()
{
    struct cstr v1, v2, v3, v4;
    cstr_ctor(&v1);
    cstr_sctor(&v2, "helloworld");
    cstr_cctor(&v3, &v2);
    cstr_ctor(&v4);
    assert(cstr_size(&v1) == 0);
    assert(cstr_strlen(&v1) == 0);
    assert(cstr_empty(&v1));
    assert(strlen(cstr_getstr(&v1)) == 0);
    assert(cstr_size(&v2) == 10);
    assert(cstr_strlen(&v2) == 10);
    assert(!cstr_empty(&v2));
    assert(strlen(cstr_getstr(&v2)) == 10);
    assert(strcmp(cstr_getstr(&v2), "helloworld") == 0);
    assert(strlen(cstr_getstr(&v3)) == 10);
    assert(strcmp(cstr_getstr(&v3), "helloworld") == 0);
    cstr_copy(&v4, &v2);
    assert(strlen(cstr_getstr(&v2)) == 10);
    assert(strcmp(cstr_getstr(&v2), "helloworld") == 0);
    assert(strlen(cstr_getstr(&v4)) == 10);
    assert(strcmp(cstr_getstr(&v4), "helloworld") == 0);
    cstr_strcpy(&v1, "abcdefg");
    assert(strlen(cstr_getstr(&v1)) == 7);
    assert(strcmp(cstr_getstr(&v1), "abcdefg") == 0);
    cstr_move(&v1, &v2);
    assert(strlen(cstr_getstr(&v1)) == 10);
    assert(strcmp(cstr_getstr(&v1), "helloworld") == 0);
    assert(cstr_size(&v2) == 0);
    assert(cstr_strlen(&v2) == 0);
    assert(cstr_empty(&v2));
    cstr_clear(&v1);
    assert(cstr_size(&v1) == 0);
    assert(cstr_strlen(&v1) == 0);
    assert(cstr_empty(&v1));
    cstr_strcpy(&v1, "123456");
    cstr_swap(&v1, &v3);
    assert(strlen(cstr_getstr(&v1)) == 10);
    assert(strcmp(cstr_getstr(&v1), "helloworld") == 0);
    assert(strlen(cstr_getstr(&v3)) == 6);
    assert(strcmp(cstr_getstr(&v3), "123456") == 0);
    cstr_strcat(&v1, "ABCDE");
    assert(strlen(cstr_getstr(&v1)) == 15);
    assert(strcmp(cstr_getstr(&v1), "helloworldABCDE") == 0);
    cstr_clear(&v1);
    cstr_strcpy(&v1, "abc");
    cstr_strncat(&v1, "ABCD", 0);
    cstr_strncat(&v1, "ABCD", 1);
    cstr_strncat(&v1, "ABCD", 2);
    cstr_strncat(&v1, "ABCD", 3);
    cstr_strncat(&v1, "ABCD", 4);
    cstr_strncat(&v1, "ABCD", 5);
    cstr_strncat(&v1, "ABCD", 6);
    assert(strcmp(cstr_getstr(&v1), "abcAABABCABCDABCDABCD") == 0);
    cstr_pushback(&v1, 'x');
    assert(strcmp(cstr_getstr(&v1), "abcAABABCABCDABCDABCDx") == 0);
    assert(cstr_front(&v1) == 'a');
    assert(cstr_back(&v1) == 'x');
    assert(cstr_at(&v1, 3) == 'A');
    cstr_popback(&v1);
    assert(strcmp(cstr_getstr(&v1), "abcAABABCABCDABCDABCD") == 0);
    cstr_shrink(&v1);
    assert(strcmp(cstr_getstr(&v1), "abcAABABCABCDABCDABCD") == 0);
    cstr_format(&v1, "%d%s%d", 123456, "AbCdEfGhIjKl", 789012);
    assert(strcmp(cstr_getstr(&v1), "123456AbCdEfGhIjKl789012") == 0);
    cstr_pop(&v1, 10);
    assert(strcmp(cstr_getstr(&v1), "123456AbCdEfGh") == 0);
    cstr_push(&v1, "aaa", 3);
    assert(strcmp(cstr_getstr(&v1), "123456AbCdEfGhaaa") == 0);
    cstr_trunc(&v1, 3);
    assert(strcmp(cstr_getstr(&v1), "123") == 0);
    cstr_fclear(&v2);
    cstr_strcpy(&v1, "abc");
    cstr_strcpy(&v2, "");
    cstr_strcpy(&v3, "def");
    cstr_cpush(&v1, &v2);
    assert(strcmp(cstr_getstr(&v1), "abc") == 0);
    cstr_cpush(&v1, &v3);
    assert(strcmp(cstr_getstr(&v1), "abcdef") == 0);
    strcpy(cstr_getbuffer(&v1, 0), "123");
    cstr_commitbuffer(&v1);
    assert(strcmp(cstr_getstr(&v1), "123") == 0);
    strcpy(cstr_getbuffer(&v2, 100), "123456789012345678901234567890");
    cstr_discardbuffer(&v2);
    assert(strcmp(cstr_getstr(&v2), "") == 0);
    cstr_dtor(&v1);
    cstr_dtor(&v2);
    cstr_dtor(&v3);
    cstr_dtor(&v4);
}
void bvec_wstr_selftest()
{
    struct wstr v1, v2, v3, v4;
    wstr_ctor(&v1);
    wstr_sctor(&v2, L"helloworld");
    wstr_cctor(&v3, &v2);
    wstr_ctor(&v4);
    assert(wstr_size(&v1) == 0);
    assert(wstr_wcslen(&v1) == 0);
    assert(wstr_empty(&v1));
    assert(wcslen(wstr_getwcs(&v1)) == 0);
    assert(wstr_size(&v2) == 10);
    assert(wstr_wcslen(&v2) == 10);
    assert(!wstr_empty(&v2));
    assert(wcslen(wstr_getwcs(&v2)) == 10);
    assert(wcscmp(wstr_getwcs(&v2), L"helloworld") == 0);
    assert(wcslen(wstr_getwcs(&v3)) == 10);
    assert(wcscmp(wstr_getwcs(&v3), L"helloworld") == 0);
    wstr_copy(&v4, &v2);
    assert(wcslen(wstr_getwcs(&v2)) == 10);
    assert(wcscmp(wstr_getwcs(&v2), L"helloworld") == 0);
    assert(wcslen(wstr_getwcs(&v4)) == 10);
    assert(wcscmp(wstr_getwcs(&v4), L"helloworld") == 0);
    wstr_wcscpy(&v1, L"abcdefg");
    assert(wcslen(wstr_getwcs(&v1)) == 7);
    assert(wcscmp(wstr_getwcs(&v1), L"abcdefg") == 0);
    wstr_move(&v1, &v2);
    assert(wcslen(wstr_getwcs(&v1)) == 10);
    assert(wcscmp(wstr_getwcs(&v1), L"helloworld") == 0);
    assert(wstr_size(&v2) == 0);
    assert(wstr_wcslen(&v2) == 0);
    assert(wstr_empty(&v2));
    wstr_clear(&v1);
    assert(wstr_size(&v1) == 0);
    assert(wstr_wcslen(&v1) == 0);
    assert(wstr_empty(&v1));
    wstr_wcscpy(&v1, L"123456");
    wstr_swap(&v1, &v3);
    assert(wcslen(wstr_getwcs(&v1)) == 10);
    assert(wcscmp(wstr_getwcs(&v1), L"helloworld") == 0);
    assert(wcslen(wstr_getwcs(&v3)) == 6);
    assert(wcscmp(wstr_getwcs(&v3), L"123456") == 0);
    wstr_wcscat(&v1, L"ABCDE");
    assert(wcslen(wstr_getwcs(&v1)) == 15);
    assert(wcscmp(wstr_getwcs(&v1), L"helloworldABCDE") == 0);
    wstr_clear(&v1);
    wstr_wcscpy(&v1, L"abc");
    wstr_wcsncat(&v1, L"ABCD", 0);
    wstr_wcsncat(&v1, L"ABCD", 1);
    wstr_wcsncat(&v1, L"ABCD", 2);
    wstr_wcsncat(&v1, L"ABCD", 3);
    wstr_wcsncat(&v1, L"ABCD", 4);
    wstr_wcsncat(&v1, L"ABCD", 5);
    wstr_wcsncat(&v1, L"ABCD", 6);
    assert(wcscmp(wstr_getwcs(&v1), L"abcAABABCABCDABCDABCD") == 0);
    wstr_pushback(&v1, L'x');
    assert(wcscmp(wstr_getwcs(&v1), L"abcAABABCABCDABCDABCDx") == 0);
    assert(wstr_front(&v1) == L'a');
    assert(wstr_back(&v1) == L'x');
    assert(wstr_at(&v1, 3) == L'A');
    wstr_popback(&v1);
    assert(wcscmp(wstr_getwcs(&v1), L"abcAABABCABCDABCDABCD") == 0);
    wstr_shrink(&v1);
    assert(wcscmp(wstr_getwcs(&v1), L"abcAABABCABCDABCDABCD") == 0);
    wstr_format(&v1, L"%d%s%d", 123456, L"AbCdEfGhIjKl", 789012);
    assert(wcscmp(wstr_getwcs(&v1), L"123456AbCdEfGhIjKl789012") == 0);
    wstr_pop(&v1, 10);
    assert(wcscmp(wstr_getwcs(&v1), L"123456AbCdEfGh") == 0);
    wstr_push(&v1, L"aaa", 3);
    assert(wcscmp(wstr_getwcs(&v1), L"123456AbCdEfGhaaa") == 0);
    wstr_trunc(&v1, 3);
    assert(wcscmp(wstr_getwcs(&v1), L"123") == 0);
    wstr_fclear(&v2);
    wstr_wcscpy(&v1, L"abc");
    wstr_wcscpy(&v2, L"");
    wstr_wcscpy(&v3, L"def");
    wstr_cpush(&v1, &v2);
    assert(wcscmp(wstr_getwcs(&v1), L"abc") == 0);
    wstr_cpush(&v1, &v3);
    assert(wcscmp(wstr_getwcs(&v1), L"abcdef") == 0);
    wcscpy(wstr_getbuffer(&v1, 0), L"123");
    wstr_commitbuffer(&v1);
    assert(wcscmp(wstr_getwcs(&v1), L"123") == 0);
    wcscpy(wstr_getbuffer(&v2, 100), L"123456789012345678901234567890");
    wstr_discardbuffer(&v2);
    assert(wcscmp(wstr_getwcs(&v2), L"") == 0);
    wstr_dtor(&v1);
    wstr_dtor(&v2);
    wstr_dtor(&v3);
    wstr_dtor(&v4);
}
#endif
