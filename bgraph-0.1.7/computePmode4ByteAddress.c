/*  computePmode4ByteAddress.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "Primitive.h"


const byte bgraph_singleBitMasks[8] = { 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 };


asm void
bgraph_computePmode4ByteAddress(void)
{
    asm
    {
        lsrb
        lsrb
        lsrb                            ; B = (horiz_coord >> 3) = horiz_coord divided by 8 pixels per byte
        pshs    b
;
; Here, A contains the vertical coordinate. By clearing B, D becomes the vertical coordinate times 256.
        clrb                            ; D = ((word) vertical_coord << 8)
        lsra                            ; shift D right 3 times
        rorb
        lsra
        rorb
        lsra
        rorb                            ; D = ((word) vertical_coord << 5) = vertical_coord times 32 bytes per row
;
        addb    ,s+
        adca    #0                      ; D = offset into the 6k buffer
        leax    d,x                     ; X => byte to change
    }
}
