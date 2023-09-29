/*  getSG12PixelAddress.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "semigr.h"


extern const byte sgMasks[2][2];
extern const unsigned char divBy3Table[48];
extern const unsigned times6times32[16];
extern const word shiftLeftBy5[12];


void
getSG12PixelAddress(byte *screenBuffer, byte x, byte y, byte **byteAddress, byte *targetBitMask)
{
    byte yDivBy6 = divBy3Table[y >> 1];  // y / 6 = y / 2 / 3
    byte yMod6 = y - yDivBy6 * 6;
    *byteAddress = screenBuffer + times6times32[yDivBy6] + (x >> 1) + shiftLeftBy5[yMod6];
    *targetBitMask = sgMasks[(y % 6) >= 3][x & 1];
}
