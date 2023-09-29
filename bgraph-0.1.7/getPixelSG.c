/*  getPixelSG.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "semigr.h"


// Register X: Points to screen byte to change.
// Register B: Byte with single bit set, indicating bit to change.
//
asm byte
getPixelSG(/*const byte *p, byte targetBitMask*/)
{
    #if 0
    byte b = *p;
    if (b & targetBitMask)  // if target pixel is set
        return ((b & 0b01110000) >> 4) + 1;  // 1..9
    return 0;
    #else
    asm
    {
        pshs    b
    ; 0,S = targetBitMask
        ldb     ,x              ; load screen byte
        bitb    ,s+             ; AND with targetBitMask; do not change B
        beq     @targetPixelNotSet
        andb    #$70            ; keep color field only
        lsrb
        lsrb
        lsrb
        lsrb
        incb
        bra     @done
@targetPixelNotSet
        clrb
@done                           ; return value in B
    }
    #endif
}
