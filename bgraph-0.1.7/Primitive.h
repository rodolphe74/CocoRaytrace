/*  Primitive.h
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#ifndef _Primitive_h_
#define _Primitive_h_

#include <coco.h>


typedef void (*Primitive_setPixelFuncPtr)(word x, byte y, byte color, void *cookie);

typedef byte (*Primitive_getPixelFuncPtr)(word x, byte y, void *cookie);


// Set a pixel in a PMODE 4 buffer.
// Can be used by Primitive_line(), Primitive_circle() and Primitive_ellipse().
// x: 0..255.
// y: 0..191.
// color: 0 or 1.
// pmode4Buffer: Start of 6k screen buffer.
//
void Primitive_setPixelPmode4(word x, byte y, byte color, void *pmode4Buffer);


// Get a pixel from a PMODE 4 buffer.
// x: 0..255.
// y: 0..191.
// pmode4Buffer: Start of 6k screen buffer.
// Returns a PMODE 4 color index, i.e., 0..1.
//
byte Primitive_getPixelPmode4(word x, byte y, void *pmode4Buffer);


// Computes the bytes to be ANDed and ORed against a PMODE 4 screen byte
// to set pixel (x, y) to the specified color.
// Used by Primitive_setPixelPmode4().
// x: 0..255.
// y: 0..191.
// color: 0 or 1.
// pmode4Buffer: Start of 6k screen buffer.
// bytePtr: *bytePtr becomes the address of the screen byte to change.
// Example:
//   byte *p, am, om;
//   Primitive_getAddressAndMaskPmode4(128, 96, 1, 0x0E00, &p, &am, &om);
//   *p = *p & am | om;
//
void Primitive_getAddressAndMaskPmode4(word x, byte y, byte color, void *pmode4Buffer, byte **bytePtr, byte *andMask, byte *orMask);


// x: 0..255.
// y: Must not exceed the limit of the graphics mode used.
// color: 0..15.
// screenBuffer: Buffer of four-bit pixels, 256 pixels per row.
//
void Primitive_setPixel256x16(word x, byte y, byte color, void *screenBuffer);

// x: 0..319.
// y: Must not exceed the limit of the graphics mode used.
// color: 0..15.
// screenBuffer: Buffer of four-bit pixels, 320 pixels per row.
//
void Primitive_setPixel320x16(word x, byte y, byte color, void *screenBuffer);


// x: 0..255.
// y: Must not exceed the limit of the graphics mode used.
// screenBuffer: Buffer of four-bit pixels, 256 pixels per row.
// Return color (0..15).
//
byte Primitive_getPixel256x16(word x, byte y, void *screenBuffer);


// x: 0..319.
// y: Must not exceed the limit of the graphics mode used.
// screenBuffer: Buffer of four-bit pixels, 320 pixels per row.
// Return color (0..15).
//
byte Primitive_getPixel320x16(word x, byte y, void *screenBuffer);


// Draw a line from (x0, y0) (inclusively) to (x1, y1) inclusively,
// in the given color.
// color, cookie: Passed to setPixel(), which will be called for each pixel to be set.
//
// Example:
//   Primitive_setPixelPmode4(0, 0, 255, 191, 1, Primitive_setPixelPmode4, (void *) 0x0E00);
// 0x0E00 is assumed to be the address of the PMODE 4 screen buffer to draw in:
// In this example, the cookie is used to pass the screen address, and Primitive_setPixelPmode4()
// will receive it as its pmode4Buffer parameter.
//
void Primitive_line(word x0, byte y0, word x1, byte y1, byte color,
                    Primitive_setPixelFuncPtr setPixel, void *cookie);


// Draw a frame from top left corner (x0, y0) (inclusively) to bottom right corner
// (x1, y1) inclusively, in the given color.
// color, cookie: Passed to setPixel(), which will be called for each pixel to be set.
//
void Primitive_rectangle(word x0, byte y0, word x1, byte y1, byte color,
                         Primitive_setPixelFuncPtr setPixel, void *cookie);


// Draw a filled rectangle from top left corner (x0, y0) (inclusively) to bottom right corner
// (x1, y1) inclusively, in the given color.
// color, cookie: Passed to setPixel(), which will be called for each pixel to be set.
//
void Primitive_filledRectangle(word x0, byte y0, word x1, byte y1, byte color,
                         Primitive_setPixelFuncPtr setPixel, void *cookie);


// Draws a circle. See Primitive_ellipse() about the parameters.
//
#define Primitive_circle(x, y, radius, color, setPixel, cookie) \
            (Primitive_ellipse((x), (y), (radius), (color), 256, 256, (setPixel), (cookie)))


// Draw an ellipse centered at (x, y), of the given radius, in the given color,
// with the given horizontal and vertical stretch factors.
// These two factors must be expression in 256ths.
// For example, 256 means 1, i.e., no stretching, while 512 means to double the size,
// 128 means to halve it, etc.
// color, cookie: Passed to setPixel(), which will be called for each pixel to be set.
//
void Primitive_ellipse(word x, byte y, byte radius, byte color,
                       word horizontalFactor, word verticalFactor,
                       Primitive_setPixelFuncPtr setPixel, void *cookie);


#endif  /* _Primitive_h_ */
