/*  getSG8PixelAddress.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "semigr.h"


extern const byte sgMasks[2][2];
extern const word shiftLeftBy5[12];


static const word shiftLeftBy7[16] =
{
    0 << 7,  1 << 7,  2 << 7,  3 << 7,  4 << 7,  5 << 7,  6 << 7,  7 << 7,
    8 << 7,  9 << 7, 10 << 7, 11 << 7, 12 << 7, 13 << 7, 14 << 7, 15 << 7
};


void
getSG8PixelAddress(byte *screenBuffer, byte x, byte y, byte **byteAddress, byte *targetBitMask)
{
    *byteAddress = screenBuffer + shiftLeftBy7[y >> 2] + (x >> 1) + shiftLeftBy5[y & 3];
    *targetBitMask = sgMasks[(y & 3) >> 1][x & 1];
}
