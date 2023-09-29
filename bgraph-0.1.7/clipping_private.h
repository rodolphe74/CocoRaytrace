/*  clipping_private.h - Private definitions for the clipping algorithm.
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#ifndef _H_clipping_private
#define _H_clipping_private

#include "clipping.h"


// Bit field values returned by compOutCode().
//
enum
{
    INSIDE = 0,
    TOP = 1,
    BOTTOM = 2,
    LEFT = 4,
    RIGHT = 8,
};


byte compOutCode(short x, short y, const ShortClipRect *rect);


#endif  /* _H_clipping_private */
