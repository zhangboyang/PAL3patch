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
    bvec_vpush(v, src);
}
void bvec_dtor(struct bvec *v)
{
    free(v->begin);
#ifdef BVEC_DEBUG
    memset(v, BVEC_FILLBYTE, sizeof(struct bvec));
#endif
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
void bvec_vpush(struct bvec *dst, const struct bvec *src)
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
void CONCAT3(clsname, _, clear)(struct clsname *s) \
{ \
    tchar t = 0; \
    bvec_clear(&s->v); \
    bvec_tpush(&s->v, &t, 1, tchar); \
} \
void CONCAT3(clsname, _, fclear)(struct clsname *s) \
{ \
    CONCAT3(clsname, _, dtor)(s); \
    CONCAT3(clsname, _, ctor)(s); \
} \
void CONCAT3(clsname, _, copy)(struct clsname *dst, const struct clsname *src) \
{ \
    bvec_copy(&dst->v, &src->v); \
} \
void CONCAT3(clsname, _, move)(struct clsname *dst, struct clsname *src) \
{ \
    bvec_move(&dst->v, &src->v); \
} \
void CONCAT3(clsname, _, swap)(struct clsname *a, struct clsname *b) \
{ \
    bvec_swap(&a->v, &b->v); \
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
    tchar t = 0; \
    size_t l; \
    bvec_tpopback(&s->v, tchar); \
    for (l = 0; l < n && str[l]; l++); \
    bvec_tpush(&s->v, str, l, tchar); \
    bvec_tpush(&s->v, &t, 1, tchar); \
} \
void CONCAT3(clsname, _, pushback)(struct clsname *s, tchar c) \
{ \
    tchar t = 0; \
    *CONCAT3(clsname, _, end)(s) = c; \
    bvec_tpush(&s->v, &t, 1, tchar); \
} \
void CONCAT3(clsname, _, popback)(struct clsname *s) \
{ \
    CONCAT3(clsname, _, end)(s)[-1] = 0; \
    bvec_tpopback(&s->v, tchar); \
} \
tchar *CONCAT3(clsname, _, getbuffer)(struct clsname *s, size_t size) \
{ \
    /* it's only vaild to call:    */ \
    /*   commitbuffer()            */ \
    /*   discardbuffer()           */ \
    /*   dtor()                    */ \
    /* after calling this function */ \
    bvec_tresize(&s->v, size, tchar); \
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
int CONCAT3(clsname, _, format)(struct clsname *s, const tchar *fmt, ...) \
{ \
    int ret; \
    size_t size = BVEC_STRING_DEFAULT_FORMATBUFFER; \
    va_list ap; \
    va_start(ap, fmt); \
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
    } \
    CONCAT3(clsname, _, commitbuffer)(s); \
     \
    va_end(ap); \
    return ret; \
} \
// TEMPLATE END

BVEC_STRING_IMPL(wstr, wchar_t, wcs, wprintf)
BVEC_STRING_IMPL(cstr, char, str, printf)
