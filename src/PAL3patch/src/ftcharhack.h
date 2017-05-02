#ifndef PAL3PATCH_FTCHARHACK_H
#define PAL3PATCH_FTCHARHACK_H

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
