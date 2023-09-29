/*  getPixelPmode4.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "bgraph_private.h"


asm byte
Primitive_getPixelPmode4(word x, byte y, void *pmode4Buffer)
{
    asm
    {
        ldb     3,s                 ; low byte of x argument
        lda     5,s                 ; y argument
        ldx     6,s                 ; pmode4Buffer
        lbsr    bgraph_computePmode4ByteAddress     ; makes X reg point to targeted byte
        ldb     ,x
;
        leax    :bgraph_singleBitMasks
        lda     3,s                 ; low byte of 'x' argument
        anda    #7
        andb    a,x         ; test bit of pixel against mask (a,x contains bgraph_singleBitMasks[x & 7])
; B is now 0 or has a single bit set
        beq     @return     ; if B is 0, return that
        ldb     #1          ; the screen bit is set, so return 1
@return
    }
}
