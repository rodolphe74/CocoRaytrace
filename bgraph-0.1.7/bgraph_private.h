/*  bgraph_private.h
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#ifndef _H_bgraph_private
#define _H_bgraph_private

#include "Primitive.h"


// bgraph_singleBitMasks[i] == (0x80 >> i), for i in 0..7.
// i.e., bgraph_singleBitMasks[i] is a byte where only bit i is set.
//
extern const byte bgraph_singleBitMasks[8];


// Input: Register A = vert. coord. (0..191),
//                 B = horiz. coord. (0..255).
//                 X => PMODE 4 buffer.
// Output: Register X => Byte targeted by the input coordinates.
//         Trashes registers A and B.
//
void bgraph_computePmode4ByteAddress(void);


byte *bgraph_compute16ColorModeByteAddress(word x, byte y, const void *screenBuffer, byte numBytesPerRow);


byte Primitive_getPixel16ColorMode(word x, byte y, void *screenBuffer, byte numBytesPerRow);


void Primitive_setPixel16ColorMode(word x, byte y, byte color, void *screenBuffer, byte numBytesPerRow);


#endif  /* _H_bgraph_private */
