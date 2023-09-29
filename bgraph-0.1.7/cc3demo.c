/*  cc3demo.c - Demo of a CoCo 3 graphics program, written in C, that accesses the screen through bank switching.

    By Pierre Sarrazin <http://sarrazip.com/>.
    This file is in the public domain.

    This program requires CC3GRAPH.DRV to be in drive 0 of the floppy disk
    that contains the binary of his program.

    To create your own CoCo 3 drawing program:
    
    - Make a copy of this file under another name, in another directory.
    - Change the contents of mainDrawingFunction().
    - Compile BGraph in its directory, to create the graphics driver (cc3graph.drv).
    - Compile this file and link it with the BGraph library:
        cmoc -o yourprog.bin -I /the/bgragh/dir yourprog.c -L /the/bgragh/dir -lbgraph
    - Put yourprog.bin and cc3graph.drv on the same floppy.
    - Do LOADM"YOURPROG":EXEC on a CoCo 3.
*/

#ifndef DRIVER_ENTRY
#error "DRIVER_ENTRY must be defined to a C hex address (e.g., 0xC000)."
#endif

#include "cc3graph.h"  /* BGraph */


// RGB approximation of the 16 colors specified by the Berkeley Logo User Manual
// for the SETPENCOLOR command:
//	 0  black	 1  blue	 2  green	 3  cyan
//	 4  red		 5  magenta	 6  yellow	 7 white
//	 8  brown	 9  tan		10  forest	11  aqua
//	12  salmon	13  purple	14  orange	15  grey
//
const byte ucbLogoPaletteValues[16] =
{
     0,  9, 18, 27,
    36, 45, 54, 63,
    34, 53, 20, 29,
    60, 40, 38, 56,
};


// Must be initialized by runGraphicsDriver().
//
GraphicsDriver *graphicsDriver;


// Must not return.
//
void
mainDrawingFunction(void)
{
    * (byte *) 65497 = 0;  // high speed, now that disk operations are done

    byte bgColor = 13;  // purple (as per ucbLogoPaletteValues[])
    graphicsDriver->clearGraphicsScreen(bgColor);

    asm { sync }  // wait for v-sync to change the palette and graphics mode

    memcpy((void *) 0xFFB0, ucbLogoPaletteValues, 16);  // assumes RGB monitor

    graphicsDriver->showGraphicsMode();

    // Draw a big rectangle.
    //
    byte rectColor = 6;
    graphicsDriver->rectangle(19, 19, 300, 180, rectColor);

    // Inside the rectangle, draw diagonal lines whose color depends on the timer.
    //
    word *timerPtr = graphicsDriver->getTimerAddress();
    for (word x = 20; x < 300; x += 1)
    {
        byte color = (((byte) (*timerPtr >> 6)) & 0x0F) ^ 8;
        graphicsDriver->line(x, 20, 319 - x, 179, color);
        byte y = 20 + (byte) ((word) (x - 20) * 160 / 280);
        graphicsDriver->line(20, y, 299, 199 - y, color);
    }

    // Draw wide ellipses in the center.
    //
    for (byte i = 0; i < 5; ++i)
        graphicsDriver->ellipse(160, 100, 18 + i * 18, 3, 420, 256);

    // Print 2 lines in the rectangle, near the bottom edge.
    //
    graphicsDriver->printText( 3, 21, "Transparent",  7, 255);   // white text, transparent background
    graphicsDriver->printText(25, 21, "   Opaque   ", 7, 4);  // white text on bg passed to clearGraphicsScreen()

    // Print a sentence in the rectangle, near its top edge.
    //
    const char *words[] = { "Pixel", "precise", "vertical", "positioning." };
    byte textCol = 4, pixelRow = 26;
    for (byte i = 0; i < sizeof(words) / sizeof(words[0]); ++i)
    {
        const char *w = words[i];
        graphicsDriver->printTextAtPixelRow(textCol, pixelRow, w, 7, 0);
        textCol += (byte) strlen(w);
        pixelRow ^= 1;  // move up or down 1 pixel row
    }

    // Draw a filled rectangle near the bottom of the screen.
    //
    graphicsDriver->filledRectangle(142, 194, 178, 199, 8);

    // Use a DRAW-like command over the filled rectangle.
    //
    graphicsDriver->draw("C7BM148,198E3F3E3F3E3F3E3F3");

    graphicsDriver->paint(319, 199, 1, bgColor);  // fill outside with blue

    * (byte *) 0xFF9A = 60;  // change border color

    // Do not return.
    //
    for (;;)
        ;
}


// Initializes graphicsDriver.
// This function trashes part of the Color Basic interpreter.
// Only returns upon error, and the interrupts are masked in this case.
// Upon success, interrupts are enabled and mainDrawingFunction() is called.
// mainDrawingFunction() must NOT return.
//
void
runGraphicsDriver(void)
{
    // Load the graphics driver on top of SECB at DRIVER_ENTRY.
    // Use Disk Basic's I/O Buffer #0 at $600 as a work buffer used by this disk operation.
    // Interrupts are disabled after this call.
    // The IRQ vector will then point to an ISR in the driver.
    //
    size_t size;
    if (readDECBFile(DRIVER_ENTRY, 0, "CC3GRAPHDRV", 0x0600, &size) != 0)
        return;  // failed to find or load the file

    // Call the code at the start of the driver. That is main() in cc3graph.c.
    // This call will obtain the address of 'theGraphicsDriver' in cc3graph.c.
    //
    // No local variable or function parameters can be used beyond the following call.
    //
    graphicsDriver = ((GraphicsDriver *(*)(void)) DRIVER_ENTRY)();

    graphicsDriver->initDriver(mainDrawingFunction);
}


int
main()
{
    initCoCoSupport();
    if (!isCoCo3)
    {
        printf("COCO 3 REQUIRED.\n");
        return 0;
    }

    rgb();
    cls(255);

    locate(6, 2);
    printf("\xBF \xBF \xBF WARNING \xBF \xBF \xBF\n"  // red blocks
           "\n"
           "\n"
           "   THE UGLINESS OF THE SCREEN\n"
           "   THAT FOLLOWS IS DUE TO THE\n"
           "   AUTHOR OF THE DEMO, NOT TO\n"
           "   THE BGRAPH LIBRARY.\n"
           "\n"
           "\n"
           "\n"
           "   LOADING DRIVER...\n"
           "   (TAKES ABOUT 7 SECONDS...)\n");

    runGraphicsDriver();
    for (;;);  // freeze upon error
    return 0;  // avoids compiler warning
}
