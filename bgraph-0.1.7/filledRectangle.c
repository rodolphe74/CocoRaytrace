/*  filledRectangle.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "Primitive.h"


void
Primitive_filledRectangle(word x0, byte y0, word x1, byte y1, byte color,
                    Primitive_setPixelFuncPtr setPixel, void *cookie)
{
    for (byte y = y0; y <= y1; ++y)
        Primitive_line(x0, y, x1, y, color, setPixel, cookie);
}
