/*  bgtst.c - Demo of a CoCo 3 graphics program, written in C, that accesses the screen through bank switching.

    By Pierre Sarrazin <http://sarrazip.com/>.
    This file is in the public domain.

    This program requires CC4GRAPH.DRV to be in drive 0 of the floppy disk
    that contains the binary of his program.

    To create your own CoCo 3 drawing program:

    - Make a copy of this file under another name, in another directory.
    - Change the contents of mainDrawingFunction().
    - Compile BGraph in its directory, to create the graphics driver (cc4graph.drv).
    - cc4graph.drv is a copy of cc3graphdrv original driver adding setPx and getPx functions
    - Compile this file and link it with the BGraph library:
        cmoc -o yourprog.bin -I /the/bgragh/dir yourprog.c -L /the/bgragh/dir -lbgraph
    - Put yourprog.bin and cc4graph.drv on the same floppy.
    - Do LOADM"YOURPROG":EXEC on a CoCo 3.
*/

#ifndef DRIVER_ENTRY
#error "DRIVER_ENTRY must be defined to a C hex address (e.g., 0xC000)."
#endif

#include "bgraph-0.1.7/cc4graph.h"  /* BGraph */


// simili DawnBringer palette
const byte ucbLogoPaletteValues[16] = {
    0, 4, 14, 7, 34, 20, 38, 56,
    25, 53, 28, 23, 52, 27, 48, 63
};


// Must be initialized by runGraphicsDriver().
GraphicsDriver *graphicsDriver;


// Must not return.
void
mainDrawingFunction(void)
{
    * (byte *) 65497 = 0;  // high speed, now that disk operations are done
    byte bgColor = 13;  // purple (as per ucbLogoPaletteValues[])
    graphicsDriver->clearGraphicsScreen(bgColor);

    asm { sync }  // wait for v-sync to change the palette and graphics mode
    memcpy((void *) 0xFFB0, ucbLogoPaletteValues, 16);  // assumes RGB monitor

    graphicsDriver->showGraphicsMode();

    // display 16 colors
    unsigned char count = 0;
    for (int i = 0; i < 16; i++) {
        graphicsDriver->filledRectangle(i * 16, 0, i * 16 + 16, 16, i);
    }

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
    if (readDECBFile(DRIVER_ENTRY, 0, "CC4GRAPHDRV", 0x0600, &size) != 0)
        return;  // failed to find or load the file

    // Call the code at the start of the driver. That is main() in cc3graph.c.
    // This call will obtain the address of 'theGraphicsDriver' in cc3graph.c.
    //
    // No local variable or function parameters can be used beyond the following call.
    //
    graphicsDriver = ((GraphicsDriver * (*)(void)) DRIVER_ENTRY)();

    graphicsDriver->initDriver(mainDrawingFunction);
}


int
main()
{
    initCoCoSupport();
    if (!isCoCo3) {
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
