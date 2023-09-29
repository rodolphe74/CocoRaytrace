/*  clipping.h - Algorithm to clip lines along a rectangle.
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#ifndef _H_clipping
#define _H_clipping

#include <coco.h>


// 2D coordinate made of short integers.
//
typedef struct ShortPoint
{
    short x;
    short y;
} ShortPoint;


// Rectangle on which cohenSutherlandLineClip() clips line segments.
//
typedef struct ShortClipRect
{
    ShortPoint topLeft;  // top-most and left-most valid coordinate
    ShortPoint bottomRight;  // bottom-most and right-most valid coordinate
} ShortClipRect;


// Cohen-Sutherland line clipping algorithm.
// See Foley, van Dam, Feiner, Hughes, "Computer Graphics: Principles and Practice",
// 2nd Edition, p. 113.
// Returns TRUE if the line accepted; the coordinates get clipped as needed.
// Returns FALSE if the line is rejected, i.e., it is outside the rectangle.
// Assumes that rect->topLeft and rect->bottomRight are in the right order.
// The y-axis is assumed to point down, i.e., y0 < y1 means that y0 is on TOP of y1.
//
BOOL cohenSutherlandLineClip(short *x0, short *y0,
                             short *x1, short *y1,
                             const ShortClipRect *rect);


#endif  /* _H_clipping */
