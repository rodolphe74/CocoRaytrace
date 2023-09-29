/*  setPixelSG.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "semigr.h"


// Register X: Points to screen byte to change.
// Register B: Byte with single bit set, indicating bit to change.
// Register A: Color: 0..9 (0 = black, 1 = green, ..., 8 = orange).
//
asm void
setPixelSG(/*byte *p, byte targetBitMask, byte color*/)
{
    #if 0
    byte b = *p;
    if (color != 0)
    {
        b &= 0b10001111;  // reset color field
        b |= (color - 1) << 4;  // apply new color
        b |= targetBitMask;  // set one bit
    }
    else  // black
    {
        b &= ~targetBitMask;  // reset one bit
    }
    *p = b;
    #else
    asm
    {
        pshs    b
    ; 0,S = targetBitMask
        ldb     ,x              ; load screen byte
        tsta                    ; is color zero?
        beq     @reset
        andb    #$8F            ; reset color field
        deca
        lsla
        lsla
        lsla
        lsla
        pshs    a
        orb     ,s+             ; apply new color
        orb     ,s+             ; OR with targetBitMask (popped), to set one bit
        bra     @store
@reset
        com     ,s              ; invert bits of targetBitMask
        andb    ,s+             ; reset one bit, pop targetBitMask
@store
        stb     ,x
    }
    #endif
}
