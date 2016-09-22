#include "common.h"

// convert between fRECT and RECT
void set_rect_frect(RECT *rect, const fRECT *frect)
{
    rect->left = frect->left;
    rect->top = frect->top;
    rect->right = frect->right;
    rect->bottom = frect->bottom;
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

// get maximum 4:3 rect inside an existing rect, the out rect is centered
// out_rect == frect is allowed
void get_43_frect(fRECT *out_frect, const fRECT *frect)
{
    double width = get_frect_width(frect);
    double height = get_frect_height(frect);
    double width_43, height_43;
    double width_43_shift, height_43_shift;
    
    if (width * 3 >= height * 4) {
        width_43 = height * (4.0 / 3.0);
        height_43 = height;
        width_43_shift = (width - width_43) / 2;
        height_43_shift = 0;
    } else {
        width_43 = width;
        height_43 = width * (3.0 / 4.0);
        width_43_shift = 0;
        height_43_shift = (height - height_43) / 2;
    }
    
    set_frect_ltwh(out_frect, frect->left + width_43_shift, frect->top + height_43_shift, width_43, height_43);
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
    
    new_total:   size of target space
    len_factor:  scale factor of length
    
    method:
        TR_LOW, TR_HIGH: segment is low/high-aligned, length and offset is scaled by len_factor
        TR_AUTO: length is scaled by len_factor (keep middle point), offset is scaled by (new_total / total)
    
    out_seg == seg is allowed
*/
void transform_fseg(fSEG *out_fseg, const fSEG *fseg, double total, double new_total, int method, double len_factor)
{
    double start = fseg->start, length = fseg->length;
    switch (method) {
        case TR_LOW:
            start *= len_factor;
            length *= len_factor;
            break;
        case TR_HIGH:
            start = new_total - (total - start) * len_factor;
            length *= len_factor;
            break;
        case TR_AUTO:
            start = (start + length / 2.0) * (new_total / total) - length * len_factor / 2.0;
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
    old_rect:   size of source rect space
    new_rect:   size of target rect space
    lr_method:  method of left/right translation
    tb_method:  method of top/bottom translation
    
    out_frect == frect is allowed
*/
void transform_frect(fRECT *out_frect, const fRECT *frect, const fRECT *old_frect, const fRECT *new_frect, int lr_method, int tb_method, double len_factor)
{
    fSEG lr, tb;
    set_fseg(&lr, frect->left, get_frect_width(frect));
    transform_fseg(&lr, &lr, get_frect_width(old_frect), get_frect_width(new_frect), lr_method, len_factor);
    set_fseg(&tb, frect->top, get_frect_height(frect));
    transform_fseg(&tb, &tb, get_frect_height(old_frect), get_frect_height(new_frect), tb_method, len_factor);
    set_frect_ltwh(out_frect, lr.start + new_frect->left, tb.start + new_frect->top, lr.length, tb.length);
}
