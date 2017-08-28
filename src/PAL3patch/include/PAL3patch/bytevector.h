#ifndef PAL3PATCH_BYTEVECTOR_H
#define PAL3PATCH_BYTEVECTOR_H
// PATCHAPI DEFINITIONS

#define BVECAPI PATCHAPI

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
extern BVECAPI void bvec_dtor(struct bvec *v); // destructor
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
extern BVECAPI void bvec_vpush(struct bvec *dst, const struct bvec *src);
extern BVECAPI void bvec_bpop(struct bvec *dst, size_t size);

// typed helper

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
#define bvec_tpop(v, nmemb, type) bvec_bpop((v), (nmemb) * sizeof(type))
#define bvec_tpopback(v, type) bvec_tpop((v), 1, type)


// high-level interface (value)

extern BVECAPI void bvec_push_char(struct bvec *v, char val);
extern BVECAPI void bvec_push_wchar(struct bvec *v, wchar_t val);
extern BVECAPI void bvec_push_int(struct bvec *v, int val);
extern BVECAPI void bvec_push_unsigned(struct bvec *v, unsigned val);
extern BVECAPI void bvec_push_ptr(struct bvec *v, void *val);


// high-level interface (strings)

extern BVECAPI char *bvec_getstr(struct bvec *v);
extern BVECAPI void bvec_strshrink(struct bvec *v);
extern BVECAPI void bvec_strcat(struct bvec *v, const char *str);
extern BVECAPI wchar_t *bvec_getwstr(struct bvec *v);
extern BVECAPI void bvec_wstrshrink(struct bvec *v);
extern BVECAPI void bvec_wcscat(struct bvec *v, const wchar_t *wstr);


// high-level interface (printf)

extern BVECAPI struct bvec bvec_printf(const char *fmt, ...);
extern BVECAPI void bvec_strcat_printf(struct bvec *v, const char *fmt, ...);
extern BVECAPI struct bvec bvec_wprintf(const wchar_t *fmt, ...);
extern BVECAPI void bvec_wcscat_wprintf(struct bvec *v, const wchar_t *fmt, ...);





#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#define BVEC_DEFAULT_CAPACITY 32
#define BVEC_DEFAULT_PRINTF_BUFSIZE 256

#endif
#endif
