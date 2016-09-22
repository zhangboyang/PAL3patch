#ifndef PAL3PATCH_TRANSFORM_H
#define PAL3PATCH_TRANSFORM_H

typedef struct _fRECT { // rect
    double left;
    double top;
    double right;
    double bottom;
} fRECT;

typedef struct _fSEG { // segment
    double start;
    double length;
} fSEG;

enum transform_method {
    TR_LOW,
    TR_HIGH,
    TR_AUTO,
};

// rect functions
extern void set_rect_frect(RECT *rect, const fRECT *frect);
extern void set_frect_rect(fRECT *frect, const RECT *rect);
extern double get_frect_width(const fRECT *frect);
extern double get_frect_height(const fRECT *frect);
extern void set_frect_ltrb(fRECT *frect, double left, double top, double right, double bottom);
extern void set_frect_ltwh(fRECT *frect, double left, double top, double width, double height);
extern void get_43_frect(fRECT *out_frect, const fRECT *frect);

// segment functions
extern void set_fseg(fSEG *fseg, double start, double length);

// transform functions
extern void transform_fseg(fSEG *out_fseg, const fSEG *fseg, double total, double new_total, int method, double len_factor);
extern void transform_frect(fRECT *out_frect, const fRECT *frect, const fRECT *old_frect, const fRECT *new_frect, int lr_method, int tb_method, double len_factor);

#endif
