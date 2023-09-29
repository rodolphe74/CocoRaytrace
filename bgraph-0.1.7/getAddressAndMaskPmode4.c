/*  getAddressAndMaskPmode4.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "bgraph_private.h"


void
Primitive_getAddressAndMaskPmode4(word x, byte y, byte color, void *pmode4Buffer, byte **bytePtr, byte *andMask, byte *orMask)
{
    #if 0  /* Original C code. */
    *andMask = ~bgraph_singleBitMasks[x & 7];  // all bits set except the targeted one
    *orMask = color << (7 - (x & 7));  // the only bit potentially set is the targeted one
    *bytePtr = ((byte *) pmode4Buffer) + ((word) y << 5) + ((byte) x >> 3);  /* N.B.: x <= 255 */
    #else
    byte resetMask, colorMask;
    asm
    {
        ldb     :x[1]                   ; low byte of 'x'
        andb    #7
        leax    :bgraph_singleBitMasks
        lda     b,x
        sta     [:orMask]               ; assuming color == 1
        coma                            ; now all bits set except one
        sta     [:andMask]
;
        tst     :color                  ; if color == 0, fix *orMask
        bne     @orMaskComputed
        clr     [:orMask]
@orMaskComputed
        ldb     :x[1]
        lda     :y                      ; no need for [1] as with :x, because 'y' is declared as byte
        ldx     :pmode4Buffer
        lbsr    bgraph_computePmode4ByteAddress     ; point X reg to targeted byte
        stx     [:bytePtr]
    }
    #endif
}
