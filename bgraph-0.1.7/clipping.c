/*  clipping.c - Algorithm to clip lines along a rectangle.
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "clipping_private.h"


// Assumes that rect->topLeft and rect->bottomRight are in the right order.
// The y-axis is assumed to point down, i.e., y0 < y1 means that y0 is above y1.
// Exported so that testclip.c can test it directly.
//
byte
compOutCode(short x, short y, const ShortClipRect *rect)
{
    byte code;
    if (y < rect->topLeft.y)
        code = TOP;
    else if (y > rect->bottomRight.y)
        code = BOTTOM;
    else
        code = INSIDE;
    if (x < rect->topLeft.x)
        code |= LEFT;
    else if (x > rect->bottomRight.x)
        code |= RIGHT;
    return code;
}
 

BOOL
cohenSutherlandLineClip(short *x0, short *y0,
                        short *x1, short *y1,
                        const ShortClipRect *rect)
{
    byte outCode0 = compOutCode(*x0, *y0, rect);
    byte outCode1 = compOutCode(*x1, *y1, rect);
 
    for (;;)
    {
        if (outCode0 == INSIDE && outCode1 == INSIDE)
            return TRUE;
        if (outCode0 & outCode1)
            return FALSE;  // both ends outside and in same region

        // Pick an endpoint that is outside.
        byte outCodeOut = (outCode0 != INSIDE ? outCode0 : outCode1);

        short x, y;
        if (outCodeOut & TOP)
        {
            x = *x0 + (*x1 - *x0) * (rect->topLeft.y - *y0) / (*y1 - *y0);
            y = rect->topLeft.y;
        }
        else if (outCodeOut & BOTTOM)
        {
            x = *x0 + (*x1 - *x0) * (rect->bottomRight.y - *y0) / (*y1 - *y0);
            y = rect->bottomRight.y;
        }
        else if (outCodeOut & RIGHT)
        {
            y = *y0 + (*y1 - *y0) * (rect->bottomRight.x - *x0) / (*x1 - *x0);
            x = rect->bottomRight.x;
        }
        else if (outCodeOut & LEFT)
        {
            y = *y0 + (*y1 - *y0) * (rect->topLeft.x - *x0) / (*x1 - *x0);
            x = rect->topLeft.x;
        }

        // Move outside point to edge of rectangle.
        if (outCodeOut == outCode0)
        {
            *x0 = x;
            *y0 = y;
            outCode0 = compOutCode(*x0, *y0, rect);
        }
        else
        {
            *x1 = x;
            *y1 = y;
            outCode1 = compOutCode(*x1, *y1, rect);
        }
    }
}
