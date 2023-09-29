/*  getSG24PixelAddress.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "semigr.h"


extern const byte sgMasks[2][2];
extern const unsigned char divBy3Table[48];
extern const unsigned times6times32[16];
extern const word shiftLeftBy5[12];


void
getSG24PixelAddress(byte *screenBuffer, byte x, byte y, byte **byteAddress, byte *targetBitMask)
{
    byte yDivBy12 = divBy3Table[y >> 2];  // y / 12 = y / 4 / 3
    byte yMod12 = y - yDivBy12 * 12;
    *byteAddress = screenBuffer + (times6times32[yDivBy12] << 1) + (x >> 1) + shiftLeftBy5[yMod12];
    *targetBitMask = sgMasks[yMod12 >= 6][x & 1];
}
