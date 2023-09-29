/*  rectangle.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "Primitive.h"


void
Primitive_rectangle(word x0, byte y0, word x1, byte y1, byte color,
                    Primitive_setPixelFuncPtr setPixel, void *cookie)
{
    Primitive_line(x0, y0, x1, y0, color, setPixel, cookie);  // top
    Primitive_line(x0, y1, x1, y1, color, setPixel, cookie);  // bottom
    Primitive_line(x0, y0, x0, y1, color, setPixel, cookie);  // left
    Primitive_line(x1, y0, x1, y1, color, setPixel, cookie);  // right
}
