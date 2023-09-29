/*  setPixel16ColorMode.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "bgraph_private.h"


void
Primitive_setPixel16ColorMode(word x, byte y, byte color, void *screenBuffer, byte numBytesPerRow)
{
    byte *p = bgraph_compute16ColorModeByteAddress(x, y, screenBuffer, numBytesPerRow);

    #if 0
    if (x & 1)  // if setting pixel at odd horiz. coord.
        *p = (*p & 0xF0) | color;
    else
        *p = (color << 4) | (*p & 0x0F);
    #else
    asm
    {
        ldb     [:p]        // *p
        lda     :x[1]       // LSB of 'x'
        bita    #1
        beq     @evenX
; 'x' is odd: change the low nybble of the screen byte
        andb    #$F0
        bra     @done
@evenX
        andb    #$0F
        lsl     :color
        lsl     :color
        lsl     :color
        lsl     :color      // :color is now original color shifted 4 bits left
@done
        orb     :color
        stb     [:p]
    }
    #endif
}
