#ifndef PAL3APATCH_TRANSFORM_H
#define PAL3APATCH_TRANSFORM_H
// PATCHAPI DEFINITIONS

typedef struct _fPOINT { // point
    double x;
    double y;
} fPOINT;

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
    TR_SCALE_LOW,
    TR_SCALE_HIGH,
    TR_SCALE_MID,

    // NOTE: if you want to modify this enum, pay attention to the size limit in struct uiwnd_ptag
};

// point functions
extern PATCHAPI void set_point(POINT *point, LONG x, LONG y);
extern PATCHAPI void set_fpoint(fPOINT *fpoint, double x, double y);
extern PATCHAPI void set_point_fpoint(POINT *point, const fPOINT *fpoint);
extern PATCHAPI void set_point_fpoint_round(POINT *point, const fPOINT *fpoint);
extern PATCHAPI void set_fpoint_point(fPOINT *fpoint, const POINT *point);

// rect functions
extern PATCHAPI void set_rect(RECT *rect, int left, int top, int right, int bottom);
extern PATCHAPI void set_rect_ltwh(RECT *rect, int left, int top, int width, int height);
extern PATCHAPI void set_rect_frect(RECT *rect, const fRECT *frect);
extern PATCHAPI void set_frect_rect(fRECT *frect, const RECT *rect);
extern PATCHAPI double get_frect_width(const fRECT *frect);
extern PATCHAPI double get_frect_height(const fRECT *frect);
extern PATCHAPI double get_frect_aspect_ratio(const fRECT *frect);
extern PATCHAPI void set_frect_ltrb(fRECT *frect, double left, double top, double right, double bottom);
extern PATCHAPI void set_frect_ltwh(fRECT *frect, double left, double top, double width, double height);
extern PATCHAPI void translate_frect_rel(fRECT *out_frect, const fRECT *frect, double lr, double tb);
extern PATCHAPI void scale_frect_fixlt(fRECT *out_frect, const fRECT *frect, double wf, double hf);
extern PATCHAPI void floor_frect(fRECT *out_frect, const fRECT *frect);
extern PATCHAPI void gbfrect2frect(fRECT *out_frect, const fRECT *frect);
extern PATCHAPI void frect2gbfrect(fRECT *out_frect, const fRECT *frect);
extern PATCHAPI void get_ratio_frect(fRECT *out_frect, const fRECT *frect, double ratio, int lr_method, int tb_method);
extern PATCHAPI double get_frect_min_scalefactor(const fRECT *dst_frect, const fRECT *src_frect);

// segment functions
extern PATCHAPI void set_fseg(fSEG *fseg, double start, double length);

// transform functions
extern PATCHAPI void transform_fseg(fSEG *out_fseg, const fSEG *fseg, double total, double dst_total, int method, double len_factor);
extern PATCHAPI void transform_frect(fRECT *out_frect, const fRECT *frect, const fRECT *src_frect, const fRECT *dst_frect, int lr_method, int tb_method, double len_factor);
extern PATCHAPI void transform_fpoint(fPOINT *out_fpoint, const fPOINT *fpoint, const fRECT *src_frect, const fRECT *dst_frect, int lr_method, int tb_method, double len_factor);


#ifdef PATCHAPI_EXPORTS
// INTERNAL DEFINITIONS

#endif
#endif
