/*  getPixel320x16.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "bgraph_private.h"


byte
Primitive_getPixel320x16(word x, byte y, void *screenBuffer)
{
    return Primitive_getPixel16ColorMode(x, y, screenBuffer, 160);
}
