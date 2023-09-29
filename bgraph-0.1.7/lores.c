/*  lores.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "lores.h"


static const byte resetMasks[4] = { 0xF7, 0xFB, 0xFD, 0xFE };


static byte
getLowResPixel(byte *screen, byte x, byte y, byte **bytePtr)
{
    *bytePtr = screen + ((word) (y >> 1) << 5) + (x >> 1);

    /*  (y & 1) (x & 1)	mask	index
	   0       0    0111	0
	   0       1	1011	1
	   1       0	1101	2
	   1       1	1110	3
    */
    return resetMasks[((y & 1) << 1) | (x & 1)];
}


// x: 0..63.
// y: 0..31.
//
void
reset(byte *screen, byte x, byte y)
{
    byte *screenBytePtr;
    byte mask = getLowResPixel(screen, x, y, &screenBytePtr);
    byte b = *screenBytePtr;
    if (b < 0x80)  // if not graphical character
	b = 0x80;
    else
	b &= mask;
    *screenBytePtr = b;
    //printf("(%2u, %2u): %p %u $%02x\n", x, y, screenBytePtr, index, resetMasks[index]);
}


void
set(byte *screen, byte x, byte y, byte color)
{
    byte *screenBytePtr;
    byte mask = getLowResPixel(screen, x, y, &screenBytePtr);
    byte b = *screenBytePtr;
    if (b < 0x80)  // if not graphical character
	b = 0x80;  // reset color field and all 4 pixels
    else
    {
	b &= 0x8F;  // reset (3-bit) color field
	b &= mask;  // reset targeted pixel
    }
    b |= (color - 1) << 4;  // set color field
    b |= ~mask;  // set targeted pixel
    *screenBytePtr = b;
}
