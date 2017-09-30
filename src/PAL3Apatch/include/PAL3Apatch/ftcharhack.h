#ifndef PAL3APATCH_FTCHARHACK_H
#define PAL3APATCH_FTCHARHACK_H
#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

struct ftcharhack_bitmap {
    int w, h;
    int yshift;
    const char *data;
};
struct ftcharhack {
    const char *fontname;
    int size;
    const wchar_t *charmap;
    struct ftcharhack_bitmap bitmap[];
};

extern const struct ftcharhack *charhack[];

#endif
#endif
