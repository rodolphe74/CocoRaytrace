/*  getPixelSG12.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "semigr.h"


void getSG12PixelAddress(byte *screenBuffer, byte x, byte y, byte **byteAddress, byte *targetBitMask);
byte getPixelSG(/*const byte *p, byte targetBitMask*/);


byte
getPixelSG12(byte *screenBuffer, byte x, byte y)
{
    byte *byteAddress;
    byte targetBitMask;
    getSG12PixelAddress(screenBuffer, x, y, &byteAddress, &targetBitMask);
    asm
    {
        ldx     :byteAddress
        ldb     :targetBitMask
        lbsr    _getPixelSG     ; return value received in B
    }
}
