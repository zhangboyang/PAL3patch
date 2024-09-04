#ifndef PAL3PATCH_BYTEVECTOR_H
#define PAL3PATCH_BYTEVECTOR_H
// PATCHAPI DEFINITIONS

#define BVECAPI PATCHAPI
//#define BVEC_DEBUG

#define bvec_typeof(type) type // FIXME


// byte vector definition

struct bvec {
    void *begin;
    void *end;
    void *capacity;
};

//
// NOTES:
//    if a bvec pointer is passed to function, it MUST be constructed first
//        (and MUST call destructor when no longer used)
//    if a function returns bvec, means the function will construct it
//        (that means: user MUST NOT construct it before, and MUST destruct it later)
//

// basic interface

extern BVECAPI void bvec_ctor(struct bvec *v); // constructor
extern BVECAPI void bvec_bctor(struct bvec *v, const void *data, size_t size); // copy constructor
extern BVECAPI void bvec_cctor(struct bvec *v, const struct bvec *src); // copy constructor
extern BVECAPI void bvec_dtor(struct bvec *v); // destructor
extern BVECAPI void *bvec_mdtor(struct bvec *v); // move destructor
extern BVECAPI void bvec_clear(struct bvec *v);
extern BVECAPI void bvec_fclear(struct bvec *v);
extern BVECAPI void bvec_copy(struct bvec *dst, const struct bvec *src);
extern BVECAPI void bvec_move(struct bvec *dst, struct bvec *src);
extern BVECAPI void bvec_swap(struct bvec *a, struct bvec *b);
extern BVECAPI void *bvec_bdata(const struct bvec *v);
extern BVECAPI void *bvec_bbegin(const struct bvec *v);
extern BVECAPI void *bvec_bend(const struct bvec *v);
extern BVECAPI int bvec_empty(const struct bvec *v);
extern BVECAPI size_t bvec_bsize(const struct bvec *v);
extern BVECAPI size_t bvec_bcapacity(const struct bvec *v);
extern BVECAPI void bvec_shrink(struct bvec *v);
extern BVECAPI void bvec_breserve(struct bvec *v, size_t size);
extern BVECAPI void bvec_bresize(struct bvec *v, size_t size);
extern BVECAPI void bvec_bpush(struct bvec *v, const void *data, size_t size);
extern BVECAPI void bvec_cpush(struct bvec *dst, const struct bvec *src);
extern BVECAPI void bvec_bpop(struct bvec *v, size_t size);

// typed helper

#define bvec_tctor(v, base, nmemb, type) bvec_bctor((v), (base), (nmemb) * sizeof(type))
#define bvec_tmdtor(v, type) ((bvec_typeof(type) *) bvec_mdtor(v))
#define bvec_tdata(v, type) ((bvec_typeof(type) *) bvec_bdata(v))
#define bvec_tbegin(v, type) ((bvec_typeof(type) *) bvec_bbegin(v))
#define bvec_tend(v, type) ((bvec_typeof(type) *) bvec_bend(v))
#define bvec_tat(v, n, type) (*(bvec_tbegin(v, type) + (n)))
#define bvec_tfront(v, type) (*bvec_tbegin((v), type))
#define bvec_tback(v, type) (*(bvec_tend((v), type) - 1))
#define bvec_tsize(v, type) (bvec_bsize(v) / sizeof(type))
#define bvec_treserve(v, size, type) bvec_breserve((v), (size) * sizeof(type))
#define bvec_tresize(v, size, type) bvec_bresize((v), (size) * sizeof(type))
#define bvec_tpush(v, base, nmemb, type) bvec_bpush((v), (base), (nmemb) * sizeof(type))
#define bvec_tpushback(v, ptr, type) bvec_tpush((v), (ptr), 1, type)
#define bvec_tpop(v, nmemb, type) bvec_bpop((v), (nmemb) * sizeof(type))
#define bvec_tpopback(v, type) bvec_tpop((v), 1, type)


// high-level interface (value)

extern BVECAPI void bvec_push_char(struct bvec *v, char val);
extern BVECAPI void bvec_push_wchar(struct bvec *v, wchar_t val);
extern BVECAPI void bvec_push_int(struct bvec *v, int val);
extern BVECAPI void bvec_push_unsigned(struct bvec *v, unsigned val);
extern BVECAPI void bvec_push_ptr(struct bvec *v, void *val);




// high-level interface (string template)


#define BVEC_STRING_DECL(clsname, tchar, tname) \
    struct clsname { \
        struct bvec v; \
    }; \
    extern BVECAPI void CONCAT3(clsname, _, ctor)(struct clsname *s); \
    extern BVECAPI void CONCAT3(clsname, _, cctor)(struct clsname *s, const struct clsname *src); \
    extern BVECAPI void CONCAT3(clsname, _, sctor)(struct clsname *s, const tchar *str); \
    extern BVECAPI void CONCAT3(clsname, _, dtor)(struct clsname *s); \
    extern BVECAPI tchar *CONCAT3(clsname, _, mdtor)(struct clsname *s); \
    extern BVECAPI void CONCAT3(clsname, _, clear)(struct clsname *s); \
    extern BVECAPI void CONCAT3(clsname, _, fclear)(struct clsname *s); \
    extern BVECAPI void CONCAT3(clsname, _, copy)(struct clsname *dst, const struct clsname *src); \
    extern BVECAPI void CONCAT3(clsname, _, move)(struct clsname *dst, struct clsname *src); \
    extern BVECAPI void CONCAT3(clsname, _, swap)(struct clsname *a, struct clsname *b); \
    extern BVECAPI const tchar *CONCAT4(clsname, _, get, tname)(const struct clsname *s); \
    extern BVECAPI tchar *CONCAT3(clsname, _, data)(const struct clsname *s); \
    extern BVECAPI tchar *CONCAT3(clsname, _, begin)(const struct clsname *s); \
    extern BVECAPI tchar *CONCAT3(clsname, _, end)(const struct clsname *s); \
    extern BVECAPI int CONCAT3(clsname, _, empty)(const struct clsname *s); \
    extern BVECAPI size_t CONCAT3(clsname, _, size)(const struct clsname *s); \
    extern BVECAPI void CONCAT3(clsname, _, shrink)(struct clsname *s); \
    extern BVECAPI size_t CONCAT4(clsname, _, tname, len)(const struct clsname *s); \
    extern BVECAPI void CONCAT4(clsname, _, tname, cpy)(struct clsname *s, const tchar *str); \
    extern BVECAPI void CONCAT4(clsname, _, tname, cat)(struct clsname *s, const tchar *str); \
    extern BVECAPI void CONCAT4(clsname, _, tname, ncat)(struct clsname *s, const tchar *str, size_t n); \
    extern BVECAPI void CONCAT3(clsname, _, push)(struct clsname *s, const tchar *str, size_t n); \
    extern BVECAPI void CONCAT3(clsname, _, pop)(struct clsname *s, size_t n); \
    extern BVECAPI void CONCAT3(clsname, _, cpush)(struct clsname *dst, const struct clsname *src); \
    extern BVECAPI void CONCAT3(clsname, _, trunc)(struct clsname *s, size_t n); \
    extern BVECAPI void CONCAT3(clsname, _, pushback)(struct clsname *s, tchar c); \
    extern BVECAPI void CONCAT3(clsname, _, popback)(struct clsname *s); \
    extern BVECAPI tchar *CONCAT3(clsname, _, getbuffer)(struct clsname *s, size_t size); \
    extern BVECAPI void CONCAT3(clsname, _, commitbuffer)(struct clsname *s); \
    extern BVECAPI void CONCAT3(clsname, _, discardbuffer)(struct clsname *s); \
    extern BVECAPI int CONCAT3(clsname, _, format)(struct clsname *s, const tchar *fmt, ...); \
// TEMPLATE END

#define BVEC_STRING_PRIVATE_DECL(clsname, tchar, tname) \
    extern int CONCAT3(clsname, _, vformat)(struct clsname *s, const tchar *fmt, va_list ap); \
// TEMPLATE END

BVEC_STRING_DECL(wstr, wchar_t, wcs)
BVEC_STRING_DECL(cstr, char, str)

#define wstr_at(s, n) (*(wstr_data(s) + (n)))
#define wstr_front(s) (*wstr_begin(s))
#define wstr_back(s) (*(wstr_end(s) - 1))

#define cstr_at(s, n) (*(cstr_data(s) + (n)))
#define cstr_front(s) (*cstr_begin(s))
#define cstr_back(s) (*(cstr_end(s) - 1))


// wstr wrapper

extern BVECAPI void wstr_cs2wcs(struct wstr *s, const char *cstr, UINT src_cp);
extern BVECAPI void cstr_wcs2cs(struct cstr *s, const wchar_t *wstr, UINT src_cp);
extern BVECAPI void cstr_cs2cs(struct cstr *s, const char *cstr, UINT src_cp, UINT dst_cp);


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#ifdef BVEC_DEBUG
#define BVEC_DEFAULT_CAPACITY 1
#define BVEC_STRING_DEFAULT_FORMATBUFFER 1
#else
#define BVEC_DEFAULT_CAPACITY 32
#define BVEC_STRING_DEFAULT_FORMATBUFFER 256
#endif

BVEC_STRING_PRIVATE_DECL(wstr, wchar_t, wcs)
BVEC_STRING_PRIVATE_DECL(cstr, char, str)

#endif
#endif
