/*  getPixelSG8.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "semigr.h"


void getSG8PixelAddress(byte *screenBuffer, byte x, byte y, byte **byteAddress, byte *targetBitMask);
byte getPixelSG(/*const byte *p, byte targetBitMask*/);


byte
getPixelSG8(byte *screenBuffer, byte x, byte y)
{
    byte *byteAddress;
    byte targetBitMask;
    getSG8PixelAddress(screenBuffer, x, y, &byteAddress, &targetBitMask);
    asm
    {
        ldx     :byteAddress
        ldb     :targetBitMask
        lbsr    _getPixelSG     ; return value received in B
    }
}
