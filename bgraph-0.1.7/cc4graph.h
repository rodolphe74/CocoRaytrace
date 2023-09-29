// cc3graph.h - CoCo 3 graphics driver.
//
// Provides functions that map in the 320x200 graphics screen using MMU Task #1,
// operate on that screen, then go back to MMU Task #0.
// This driver also provides a timer that gets incremented 60 times per second.
//
// SECB's HPRINT font is reused.

#ifndef _H_cc4graph
#define _H_cc4graph

#include <coco.h>


enum
{
    DRAW_BUFFER_SIZE = 256,  // maximum length of a draw() script, including terminating '\0'
};


typedef struct GraphicsDriver
{
    // Must be called once before calling the following functions.
    // Does not clear the screen.
    // Sets the stack pointer to the bottom of this driver's stack space, in a part
    // of memory that never gets tuned out.
    // Enables the interrupts.
    // Jumps to postLoadFunction, which must not return.
    //
    void (*initDriver)(void (*postLoadFunction)(void));

    // color: 0..15 (palette slot).
    //
    void (*clearGraphicsScreen)(byte color);

    void (*showGraphicsMode)(void);

	void (*setPx)(word x, byte y, byte color);
	void (*getPx)(word x, byte y, byte *color);

    // x0, x1: 0..319.
    // y0, y1: 0..199.
    // color: 0..15 (palette slot).
    //
    void (*line)(word x0, byte y0, word x1, byte y1, byte color);

    // Draws a frame.
    // x0, x1: Top left corner.
    // y0, y1: Bottom right corner.
    // color: 0..15 (palette slot).
    //
    void (*rectangle)(word x0, byte y0, word x1, byte y1, byte color);

    void (*filledRectangle)(word x0, byte y0, word x1, byte y1, byte color);

    // x: 0..319.
    // y: 0..199.
    // radius: In pixels.
    // horizontalFactor, verticalFactor: See Primitive.h.
    // color: 0..15 (palette slot).
    //
    void (*ellipse)(word x, byte y, byte radius, byte color,
                        word horizontalFactor, word verticalFactor);

    // Paints with newColor until edges of oldColor are reached.
    // x: 0..319.
    // y: 0..199.
    // newColor, oldColor: 0..15 (palette slot).
    //
    byte (*paint)(word x, byte y, byte newColor, byte oldColor);

    // See Drawer_draw() in Drawer.h.
    // script: Must be at most DRAW_BUFFER_SIZE characters, including the terminating '\0'.
    //
    void (*draw)(const char *script);

    // textColumn: 0..39.
    // textRow: 0..24.
    // textColor: 0..15 (palette slot).
    // backgroundColor: 0..15 to get an opaque background.
    //                  If a higher value is passed, the printing is transparent.
    //
    void (*printText)(byte textColumn, byte textRow, const char *text,
                      byte textColor, byte backgroundColor);

    // Like printText(), but pixelRow (0..192) specifies the vertical position
    // in pixels instead of text rows.
    //
    void (*printTextAtPixelRow)(byte textColumn, byte pixelRow, const char *text,
                                byte textColor, byte backgroundColor);

    // Returns a pointer to an unsigned integer that gets incremented
    // 60 times per second.
    //
    word *(*getTimerAddress)(void);

} GraphicsDriver;


#endif  /* _H_cc3graph */
