BGraph
======

By Pierre Sarrazin <http://sarrazip.com/>, except Primitive_line().

This library is in the public domain.

This library offers graphics functions similar to those offered by
Color Basic. It does not assume the presence of the Color Basic interpreter.


## Drawer: Supports most commands of Basic's DRAW.

Create a Drawer object, then call either Drawer_init(), Drawer_initForPMODE()
or Drawer_initForHSCREEN() on it. Note that a PMODE graphics mode can be
set up with functions pmode() and screen(), which come with the CMOC standard
library since version 0.1.53 of that compiler.

To draw, call **Drawer_draw()** with a DRAW-like string of commands.

See Drawer.h for details.


## Painter: Similar to Basic's PAINT.

Basic's PAINT requires the caller to specify the color to paint with and
the color that acts as the boundary where the flood fill stops. With the
Painter object, the latter color must instead be the color to be replaced.
For example, if a black (0) region must be painted white (1), one would
pass 1 for newColor and 0 for oldColor.


## Graphics primitives

**Primitive.h** contains a few primitive graphics operations, including
**Primitive_line()** to draw a line segment.

These primitives do not clip the shapes they draw. See the next section.


## Line clipping

To clip a line segment with respect to a rectangle:

*   \#include "clipping.h"
*   define a **ShortClipRect** to specify a rectangle;
*   call **cohenSutherlandLineClip()** with the addresses of the line segment's
    coordinates and the address of the ShortClipRect;
    if the line is at least partly inside the rectangle, TRUE will be returned
    and the coordinates will have been adjusted as needed;
    if the line is completely outside the rectangle, FALSE is returned.

The y-axis is assumed to point down, i.e., y0 < y1 means that
y0 is _above_ y1.

If a line segment to be drawn by Primitive_line() is likely to be partly off screen,
calling cohenSutherlandLineClip() beforehand will adjust the coordinates as needed,
or indicate if the line would be completely off screen.

Program testclip.bin executes a few unit tests on cohenSutherlandLineClip().


## Catmull-Rom curves

See drawCatmullRomPath() in catmull-rom.h.

For details on the algorithm, see Foley, van Dam, Feiner, Hughes,
_Computer Graphics: Principles and Practice_, 2nd Edition, pp. 483, 505.

Program cmdemo.bin is a demo of these curves in PMODE 4 graphics.
It draws such curves, or lets the user place points with the joystick
and draws a curve through them. These functions do not use
floating-point arithmetic.


## Dragon support

To compile for the Dragon, pass TARGET=dragon to make.


## Version history

    0.1.0 - 2018-09-20 - First public release.
    0.1.1 - 2018-11-25 - Added set() and reset() for 32x16 graphics (lores.h).
                         Now compilable for the Dragon (make TARGET=dragon).
    0.1.2 - 2019-08-25 - Added Primitive_circle() and Primitive_ellipse()
                        to Primitive.h.
    0.1.3 - 2020-07-18 - Added support for setting up semigraphics modes 8, 12
                         and 24 and for setting, resetting and getting pixels in
                         those modes. The sgdemo.c program shows these modes.
                         Added public functions Drawer_setPosition(),
                         Drawer_setColorIndex(), Primitive_setPixelPmode4(),
                         Primitive_getPixelPmode4(). The latter two are coded
                         in inline assembly language.
                         Added a 64x32 paint demo to the bgraph.c demo program.
    0.1.4 - 2022-01-02 - Added cc3demo, which draws on a 320x200 CoCo 3 screen.
                         Optimized Primitive_line() by rewriting it in assembly
                         language. This will also accelerate Primitive_circle()
                         and Primitive_ellipse().
    0.1.5 - 2022-01-14 - Painter_init() now accepts a callback to allow the
                         user program to stop the painting done by Painter_paint().
                         Existing programs can just pass NULL for the 4th argument
                         of Painter_init(), to compile as before.
                         See Painter.h for details.
    0.1.6 - 2022-02-19 - Call to readDECBFile() in cc3demo.c fixed for CMOC 0.1.75.
    0.1.7 - 2023-02-12 - Added cohenSutherlandLineClip(), declared in clipping.h,
                         which clips a line with respect to a rectangle.
                         Added drawCatmullRomPath(), which draws a closed
                         Catmull-Rom curve.
                         See catmull-rom.h for details and cmdemo.bin for a demo.
                         These functions do not use floating-point arithmetic.
                         Added Primitive_setPixel256x16(), Primitive_setPixel320x16()
                         and the respective getters to Primitive.h.
