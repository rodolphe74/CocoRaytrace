/*  Drawer.h
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#ifndef _H_Drawer
#define _H_Drawer

#include <coco.h>

#include "Primitive.h"


typedef struct Drawer
{
    byte *screenBegin;           // Start of screen buffer.
    byte *screenEnd;             // One byte past last byte of screen.
    word screenWidthInPixels;
    byte screenHeightInPixels;
    byte bytesPerRow;
    byte pixelsPerByte;          // Must be 1, 2, 4 or 8.
    word xPos;                   // Current horizontal pixel position.
    byte yPos;                   // Current vertical pixel position.
    byte *ptr;                   // Byte containing current pixel position.
    const byte *resetMaskArray;  // One mask for each pixel in a byte. Set bits represent pixel position.
    const byte *colorMaskArray;  // One mask for each supported color in current graphics mode.
    byte colorIndex;             // Current color, as an index into colorMaskArray[].
    byte colorMask;              // Byte where all pixels are of current color.
} Drawer;


// See struct Drawer for details.
// The initial pixel position is the center of the screen.
// pixelsPerByte: 8 (2-color mode), 4 (4-color mode) or 2 (16-color mode).
// Returns FALSE upon an invalid parameter.
//
BOOL Drawer_init(Drawer *d,
                 byte *screenBuffer,
                 word screenWidthInPixels, byte screenHeightInPixels,
                 byte pixelsPerByte);


// Simpler way to initialize a PMODE Drawer.
// screenBuffer: The second parameter passed to pmode().
// pmodeNo: The first parameter passed to pmode().
// See Drawer_init() for details.
//
BOOL Drawer_initForPMODE(Drawer *d, byte *screenBuffer, byte pmodeNo);


// Simpler way to initialize a CoCo 3 HSCREEN mode Drawer.
// modeNo: Argument that would be passed to Basic's HSCREEN command.
// See Drawer_init() for details.
//
BOOL Drawer_initForHSCREEN(Drawer *d, byte *screenBuffer, byte modeNo);


// Draws according to the given script, which follows most of Color Basic's
// DRAW command language.
// Returns:
//   Upon success, a null pointer.
//   Upon an error in 'script', a pointer to the error.
//
// CAUTION: One major difference is that maximum coordinates depend on the
//          graphics mode. For example, if 128 and 96 were passed to Drawer_init(),
//          then the coordinates must stay in 0..127 and 0..95. This is unlike
//          Basic's DRAW command, where the maximums are always 255 and 191,
//          regardless of the actual number of pixels on the screen in the current
//          graphics mode.
//
// Supported commands:
//   M              Move (absolute or relative). Like in Basic, but the maximum horizontal and
//                  vertical coordinates depend on the width and height passed to Drawer_init().
//   U, D, L, R     Up, down, left right, as in Basic.
//   E, F, G, H     North-east, south-east, south-west, north-west, as in Basic.
//   C              Followed by a color index, which serves as a index into the colorMaskArray[]
//                  passed to Drawer_init(). If Drawer_initForPMODE4() was called instead, the
//                  valid color indices are 0 for black and 1 for white/green. Note that color
//                  numbers in Basic's DRAW are interpreted differently.
//   B              Blank (no draw, just move), as in Basic.
//   N              No update of the draw position, as in Basic.
//
// Unsupported commands: A, S, X.
//
const char *Drawer_draw(Drawer *d, const char *script);


// Stores the current pixel position in *pX and *pY.
//
void Drawer_getPosition(Drawer *d, word *pX, byte *pY);


// Sets the current pixel position to (x, y).
//
void Drawer_setPosition(Drawer *d, word x, byte y);


// newColorIndex: Index into d->colorMaskArray[].
//
void Drawer_setColorIndex(Drawer *d, byte newColorIndex);


#endif  /* _H_Drawer */
