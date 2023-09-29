/*  semigr.h
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include <coco.h>


// Type of a function that sets the pixel at (x, y) to the given color
// in a screen buffer that starts at the given address.
//
typedef void (*SetPixelFunction)(byte *screenBuffer, byte x, byte y, byte color);


// Type of a function that gets the color of the pixel at (x, y)
// in a screen buffer that starts at the given address.
//
typedef byte (*GetPixelFunction)(byte *screenBuffer, byte x, byte y);


// Sets up and displays the requested Semigraphics mode so that it displays
// the pixels defined at the given screen buffer address.
// mode: 0, 8, 12 or 24. Zero restores the default Semigraphics 4 mode.
// Returns TRUE upon success, FALSE otherwise (invalid mode).
//
BOOL sgMode(byte mode, void *screenBuffer);


// x: 0..63.
// y: 0..63.
// color: 0..9 (0 = black, 1 = green, ..., 8 = orange).
//
void setPixelSG8(byte *screenBuffer, byte x, byte y, byte color);


// x: 0..63.
// y: 0..95.
// color: 0..9 (0 = black, 1 = green, ..., 8 = orange).
//
void setPixelSG12(byte *screenBuffer, byte x, byte y, byte color);


// x: 0..63.
// y: 0..191.
// color: 0..9 (0 = black, 1 = green, ..., 8 = orange).
//
void setPixelSG24(byte *screenBuffer, byte x, byte y, byte color);


// x: 0..63.
// y: 0..63.
// Returns: 0..9 (0 = black, 1 = green, ..., 8 = orange).
//
byte getPixelSG8(byte *screenBuffer, byte x, byte y);


// x: 0..63.
// y: 0..95.
// Returns: 0..9 (0 = black, 1 = green, ..., 8 = orange).
//
byte getPixelSG12(byte *screenBuffer, byte x, byte y);


// x: 0..63.
// y: 0..191.
// Returns: 0..9 (0 = black, 1 = green, ..., 8 = orange).
//
byte getPixelSG24(byte *screenBuffer, byte x, byte y);
