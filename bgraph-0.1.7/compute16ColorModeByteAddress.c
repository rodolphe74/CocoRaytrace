/*  compute16ColorModeByteAddress.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "bgraph_private.h"


asm byte *
bgraph_compute16ColorModeByteAddress(word x, byte y, const void *screenBuffer, byte numBytesPerRow)
{
    asm
    {
        lsr     2,s                 ; shift MSB of x parameter right one bit
        ror     3,s                 ; shift LSB; x is now half its original value
        ldb     5,s                 ; y parameter
        lda     9,s                 : numBytesPerRow
        mul
        addd    2,s                 ; add x/2
        addd    6,s                 ; screenBuffer
    }
}
