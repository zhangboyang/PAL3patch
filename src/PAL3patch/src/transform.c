#include "common.h"

// convert betwenn fPOINT and POINT
void set_point(POINT *point, LONG x, LONG y)
{
    point->x = x;
    point->y = y;
}
void set_fpoint(fPOINT *fpoint, double x, double y)
{
    fpoint->x = x;
    fpoint->y = y;
}
void set_point_fpoint(POINT *point, const fPOINT *fpoint)
{
    point->x = floor(fpoint->x + eps);
    point->y = floor(fpoint->y + eps);
}
void set_point_fpoint_round(POINT *point, const fPOINT *fpoint)
{
    point->x = round(fpoint->x + eps);
    point->y = round(fpoint->y + eps);
}
void set_fpoint_point(fPOINT *fpoint, const POINT *point)
{
    fpoint->x = point->x;
    fpoint->y = point->y;
}

// convert between fRECT and RECT
void set_rect_frect(RECT *rect, const fRECT *frect)
{
    rect->left = floor(frect->left + eps);
    rect->top = floor(frect->top + eps);
    
    // use ceiling
    rect->right = rect->left + ceil(get_frect_width(frect) - eps);
    rect->bottom = rect->top + ceil(get_frect_height(frect) - eps);
}
void set_frect_rect(fRECT *frect, const RECT *rect)
{
    set_frect_ltrb(frect, rect->left, rect->top, rect->right, rect->bottom);
}

double get_frect_width(const fRECT *frect)
{
    return frect->right - frect->left;
}
double get_frect_height(const fRECT *frect)
{
    return frect->bottom - frect->top;
}
double get_frect_aspect_ratio(const fRECT *frect)
{
    return get_frect_width(frect) / get_frect_height(frect);
}

// set rect by Left, Top, Right, Bottom
void set_frect_ltrb(fRECT *frect, double left, double top, double right, double bottom)
{
    frect->left = left;
    frect->top = top;
    frect->right = right;
    frect->bottom = bottom;
}

// set rect by Left, Top, Width, Height
void set_frect_ltwh(fRECT *frect, double left, double top, double width, double height)
{
    set_frect_ltrb(frect, left, top, left + width, top + height);
}

// translate (move) a rect relatively
void translate_frect_rel(fRECT *out_frect, const fRECT *frect, double lr, double tb)
{
    set_frect_ltrb(out_frect, frect->left + lr, frect->top + tb, frect->right + lr, frect->bottom + tb);
}

// scale a rect, Left and Top is fixed
void scale_frect_fixlt(fRECT *out_frect, const fRECT *frect, double wf, double hf)
{
    set_frect_ltwh(out_frect, frect->left, frect->top, get_frect_width(frect) * wf, get_frect_height(frect) * hf);
}

// convert between GameBox coord and screen coord
void gbfrect2frect(fRECT *out_frect, const fRECT *frect)
{
    set_frect_ltrb(out_frect, gbx2x(frect->left), gby2y(frect->top), gbx2x(frect->right), gby2y(frect->bottom));
}
void frect2gbfrect(fRECT *out_frect, const fRECT *frect)
{
    set_frect_ltrb(out_frect, x2gbx(frect->left), y2gby(frect->top), x2gbx(frect->right), y2gby(frect->bottom));
}

// get maximum rect with ratio (width / height) inside an existing rect
// out_frect == frect is allowed
void get_ratio_frect(fRECT *out_frect, const fRECT *frect, double ratio, int lr_method, int tb_method)
{
    double width = get_frect_width(frect);
    double height = get_frect_height(frect);
    double new_width, new_height;
    
    if (width >= height * ratio) {
        new_width = height * ratio;
        new_height = height;
    } else {
        new_width = width;
        new_height = width / ratio;
    }
    
    fRECT tmp_frect;
    set_frect_ltwh(&tmp_frect, 0, 0, new_width, new_height);
    transform_frect(&tmp_frect, &tmp_frect, frect, frect, lr_method, tb_method, 1.0);
    *out_frect = tmp_frect;
}


double get_frect_min_scalefactor(const fRECT *dst_frect, const fRECT *src_frect)
{
    return fmin(get_frect_width(dst_frect) / get_frect_width(src_frect),
                get_frect_height(dst_frect) / get_frect_height(src_frect));
}

void set_fseg(fSEG *fseg, double start, double length)
{
    fseg->start = start;
    fseg->length = length;
}


/*
    transform a segment
    
    <--------total-------->
         <length>
    [----########---------]
         ^start
    
    dst_total:   size of target space
    len_factor:  scale factor of length
    
    method:
        TR_LOW, TR_HIGH: segment is low/high-aligned, length and offset is scaled by len_factor
        TR_CENTER: segment is center-aligned, length is scaled by len_factor
        TR_SCALE_LOW: length is scaled by len_factor (keep lower point), offset is scaled by (dst_total / src_total)
        TR_SCALE_HIGH: length is scaled by len_factor (keep higher point), offset is scaled by (dst_total / src_total)
        TR_SCALE_MID: length is scaled by len_factor (keep middle point), offset is scaled by (dst_total / src_total)
    
    out_seg == seg is allowed
*/
void transform_fseg(fSEG *out_fseg, const fSEG *fseg, double src_total, double dst_total, int method, double len_factor)
{
    double start = fseg->start, length = fseg->length;
    switch (method) {
        case TR_LOW:
            start *= len_factor;
            length *= len_factor;
            break;
        case TR_HIGH:
            start = dst_total - (src_total - start) * len_factor;
            length *= len_factor;
            break;
        case TR_CENTER:
            start = (dst_total - length * len_factor) / 2.0;
            length *= len_factor;
            break;
        case TR_SCALE_LOW:
            start *= dst_total / src_total;
            length *= len_factor;
            break;
        case TR_SCALE_HIGH:
            start = (start + length) * (dst_total / src_total) - length * len_factor;
            length *= len_factor;
            break;
        case TR_SCALE_MID:
            start = (start + length / 2.0) * (dst_total / src_total) - length * len_factor / 2.0;
            length *= len_factor;
            break;
        default:
            fail("invalid segment translate method: %d", method);
    }
    out_fseg->start = start;
    out_fseg->length = length;
}


/*
    transform an fRECT
    
    out_frect:  result rect
    frect:      rect to translate
    src_rect:   size of source rect space
    dst_rect:   size of target rect space
    lr_method:  method of left/right translation
    tb_method:  method of top/bottom translation
    
    out_frect == frect is allowed
*/
void transform_frect(fRECT *out_frect, const fRECT *frect, const fRECT *src_frect, const fRECT *dst_frect, int lr_method, int tb_method, double len_factor)
{
    fSEG lr, tb;
    set_fseg(&lr, frect->left - src_frect->left, get_frect_width(frect));
    transform_fseg(&lr, &lr, get_frect_width(src_frect), get_frect_width(dst_frect), lr_method, len_factor);
    set_fseg(&tb, frect->top - src_frect->top, get_frect_height(frect));
    transform_fseg(&tb, &tb, get_frect_height(src_frect), get_frect_height(dst_frect), tb_method, len_factor);
    set_frect_ltwh(out_frect, lr.start + dst_frect->left, tb.start + dst_frect->top, lr.length, tb.length);
}


/*
    transform an fPOINT
    
    a simple wrapper of transform_frect
*/
void transform_fpoint(fPOINT *out_fpoint, const fPOINT *fpoint, const fRECT *src_frect, const fRECT *dst_frect, int lr_method, int tb_method, double len_factor)
{
    fRECT frect;
    set_frect_ltwh(&frect, fpoint->x, fpoint->y, 0.0, 0.0);
    transform_frect(&frect, &frect, src_frect, dst_frect, lr_method, tb_method, len_factor);
    set_fpoint(out_fpoint, frect.left, frect.top);
}
