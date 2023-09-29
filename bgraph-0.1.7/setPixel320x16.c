/*  setPixel320x16.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "bgraph_private.h"


void
Primitive_setPixel320x16(word x, byte y, byte color, void *screenBuffer)
{
    Primitive_setPixel16ColorMode(x, y, color, screenBuffer, 160);
}

