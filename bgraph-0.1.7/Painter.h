/*  Painter.h
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#ifndef _H_Painter
#define _H_Painter

#include <coco.h>


// x, y: Pixel position.
// cookie: The 'cookie' value passed to Painter_paint().
// Returns the color index at the given pixel position.
//
typedef byte (*Painter_getPixelFuncPtr)(word x, byte y, void *cookie);


// x, y: Pixel position.
// color: Index of the color to be given to the pixel at the given position.
// cookie: The 'cookie' value passed to Painter_paint().
//
typedef void (*Painter_setPixelFuncPtr)(word x, byte y, byte color, void *cookie);


// Function to be called periodically by Painter_paint() to determine if
// the painting operation must continue or not.
// cookie: The 'cookie' value passed to Painter_paint().
// Returns: Non-zero to continue, zero to stop.
//
typedef BOOL (*Painter_continuationFuncPtr)(void *cookie);


typedef struct Painter
{
    Painter_getPixelFuncPtr getPixel;
    Painter_setPixelFuncPtr setPixel;
    Painter_continuationFuncPtr continuationFuncPtr;
    word screenWidthInPixels;
    byte screenHeightInPixels;
    byte *stackArray;  // non null pointer
    size_t stackCapacity;  // number of bytes in stackArray[]
    size_t stackSize;  // number of bytes currently stored in stackArray[]
} Painter;


// Error codes related to this Painter object.
//
enum
{
    Painter_NO_ERROR,           // Success.
    Painter_INVALID_ARGUMENT,
    Painter_OUT_OF_STACK,
};


// getPixel: Function that returns the color index at a certain position.
// setPixel: Function that changes the color index at a certain position.
// continuationFuncPtr: Function that will be called periodically by Painter_paint()
//                      and returns non-zero to continue the painting, or zero to stop.
//                      Allowed to be NULL.
// screenWidthInPixels, screenHeightInPixels: Dimensions of the screen.
// workSpace: Temporary space to be used by the flood fill algorithm.
//            This space must remain alive as long as the Painter object is used.
// workSpaceCapacity: Number of bytes in the array designated by workSpace.
//                    64 bytes should be enough to paint relatively complex figures.
//
// Returns FALSE if one or more of the arguments is invalid (null or zero).
//
// Example:
//   Painter painter;
//   byte workSpace[64];
//   if (!Painter_init(&painter, myPixelGetter, myPixelSetter, NULL, 256, 192, stack, sizeof(workSpace)))
//       [error];
//   else
//   {
//       byte err = Painter_paint(&painter, 128, 96, 1, 0, (void *) screen);
//       if (err != Painter_NO_ERROR) [error];
//   }
//
BOOL Painter_init(Painter *p,
                  Painter_getPixelFuncPtr getPixel,
                  Painter_setPixelFuncPtr setPixel,
                  Painter_continuationFuncPtr continuationFuncPtr,
                  word screenWidthInPixels,
                  byte screenHeightInPixels,
                  byte *workSpace,
                  size_t workSpaceCapacity);


// x, y: Pixel position where to start the flood fill.
// w, h: Size of the screen in pixels.
// newColor: Color index to paint with. In a 2-color mode for example, this would be 0 or 1.
// oldColor: Index of the color to be replaced. Must not be the same as newColor.
// cookie: User-defined pointer to be passed to the getPixel and setPixel functions
//         that were passed to Painter_init(). For example, this can be the start of the
//         screen buffer, which the two functions use to know with which screen to work.
//         Allowed to be null.
//
// Returns an error code from the enum defined above (Painter_NO_ERROR upon success).
// Painter_INVALID_ARGUMENT is returned if newColor and oldColor are the same.
// Painter_OUT_OF_STACK means that the array passed to Painter_init() was too small for
// this particular paint job.
//
byte Painter_paint(Painter *p,
                   word x, byte y,
                   byte newColor, byte oldColor,
                   void *cookie);


#endif  /* _H_Painter */
