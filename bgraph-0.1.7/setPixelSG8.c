/*  setPixelSG8.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "semigr.h"


void getSG8PixelAddress(byte *screenBuffer, byte x, byte y, byte **byteAddress, byte *targetBitMask);
void setPixelSG(/*byte *p, byte targetBitMask, byte color*/);


void
setPixelSG8(byte *screenBuffer, byte x, byte y, byte color)
{
    byte *byteAddress;
    byte targetBitMask;
    getSG8PixelAddress(screenBuffer, x, y, &byteAddress, &targetBitMask);
    asm
    {
        ldx     :byteAddress
        ldb     :targetBitMask
        lda     :color
        lbsr    _setPixelSG
    }
}
