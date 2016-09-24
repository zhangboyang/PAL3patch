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
    TR_CENTER,
    TR_SCALE,
};

// rect functions
extern void set_rect_frect(RECT *rect, const fRECT *frect);
extern void set_frect_rect(fRECT *frect, const RECT *rect);
extern double get_frect_width(const fRECT *frect);
extern double get_frect_height(const fRECT *frect);
extern void set_frect_ltrb(fRECT *frect, double left, double top, double right, double bottom);
extern void set_frect_ltwh(fRECT *frect, double left, double top, double width, double height);
extern void translate_frect_rel(fRECT *out_frect, fRECT *frect, double lr, double tb);
extern void scale_frect_fixlt(fRECT *out_frect, fRECT *frect, double wf, double hf);
extern void get_43_frect(fRECT *out_frect, const fRECT *frect);
extern double get_frect_min_scalefactor(fRECT *dst_frect, fRECT *src_frect);

// segment functions
extern void set_fseg(fSEG *fseg, double start, double length);

// transform functions
extern void transform_fseg(fSEG *out_fseg, const fSEG *fseg, double total, double dst_total, int method, double len_factor);
extern void transform_frect(fRECT *out_frect, const fRECT *frect, const fRECT *src_frect, const fRECT *dst_frect, int lr_method, int tb_method, double len_factor);

#endif
