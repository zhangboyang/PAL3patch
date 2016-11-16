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
    point->x = fpoint->x;
    point->y = fpoint->y;
}
void set_fpoint_point(fPOINT *fpoint, const POINT *point)
{
    fpoint->x = point->x;
    fpoint->y = point->y;
}

// convert between fRECT and RECT
void set_rect_frect(RECT *rect, const fRECT *frect)
{
    rect->left = frect->left;
    rect->top = frect->top;
    
    // use ceiling
    rect->right = rect->left + ceil(get_frect_width(frect));
    rect->bottom = rect->top + ceil(get_frect_height(frect));
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
void translate_frect_rel(fRECT *out_frect, fRECT *frect, double lr, double tb)
{
    set_frect_ltrb(out_frect, frect->left + lr, frect->top + tb, frect->right + lr, frect->bottom + tb);
}

// scale a rect, Left and Top is fixed
void scale_frect_fixlt(fRECT *out_frect, fRECT *frect, double wf, double hf)
{
    set_frect_ltwh(out_frect, frect->left, frect->top, get_frect_width(frect) * wf, get_frect_height(frect) * hf);
}

// get maximum rect at a ratio of rwidth:rheight inside an existing rect
// the out_frect in centered in frect
// out_frect == frect is allowed
void get_ratio_frect(fRECT *out_frect, const fRECT *frect, double rwidth, double rheight)
{
    double width = get_frect_width(frect);
    double height = get_frect_height(frect);
    double new_width, new_height;
    double new_width_shift, new_height_shift;
    
    if (width * rheight >= height * rwidth) {
        new_width = height * (rwidth / rheight);
        new_height = height;
        new_width_shift = (width - new_width) / 2;
        new_height_shift = 0;
    } else {
        new_width = width;
        new_height = width * (rheight / rwidth);
        new_width_shift = 0;
        new_height_shift = (height - new_height) / 2;
    }
    
    set_frect_ltwh(out_frect, frect->left + new_width_shift, frect->top + new_height_shift, new_width, new_height);
}


double get_frect_min_scalefactor(fRECT *dst_frect, fRECT *src_frect)
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
        TR_SCALE: length is scaled by len_factor (keep middle point), offset is scaled by (dst_total / src_total)
        TR_SCALE_LOW: length is scaled by len_factor (keep lower point), offset is scaled by (dst_total / src_total)
    
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
        case TR_SCALE_CENTER:
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
