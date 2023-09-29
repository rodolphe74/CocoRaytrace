/*  getPixel16ColorMode.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "bgraph_private.h"


byte
Primitive_getPixel16ColorMode(word x, byte y, void *screenBuffer, byte numBytesPerRow)
{
    byte *bytePtr = bgraph_compute16ColorModeByteAddress(x, y, screenBuffer, numBytesPerRow);
    if (x & 1)  // if odd pixel
        return *bytePtr & 0x0F;
    return *bytePtr >> 4;  // even pixel
}
