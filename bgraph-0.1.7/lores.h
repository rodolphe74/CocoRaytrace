/*  lores.h
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#ifndef _H_lores
#define _H_lores

#include <coco.h>


// screen: Address of 32x16 text screen (0x0400 by default).
// x: 0..63.
// y: 0..31.
//
void reset(byte *screen, byte x, byte y);


// See set().
// color: 1..8.
//
void set(byte *screen, byte x, byte y, byte color);


#endif  /* _H_lores */
