/*  setPixelPmode4.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "bgraph_private.h"


void
Primitive_setPixelPmode4(word x, byte y, byte color, void *pmode4Buffer)
{
    byte *bytePtr;
    byte andMask, orMask;
    Primitive_getAddressAndMaskPmode4(x, y, color, pmode4Buffer, &bytePtr, &andMask, &orMask);
    asm  // *bytePtr = *bytePtr & andMask | colorMask;
    {
        ldx     :bytePtr
        ldb     ,x
        andb    :andMask
        orb     :orMask
        stb     ,x
    }
}
