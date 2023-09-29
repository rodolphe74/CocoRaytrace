#include "cc3graph.h"

#include "Primitive.h"
#include "Painter.h"
#include "Drawer.h"


enum
{
    // Memory map of graphics driver (in order of increasing addresses):
    // - Stack space.
    // - Painter work space.
    // - Draw buffer.
    // - Font buffer.
    // - $FE00..$FEFF.
    //
    STACK_SPACE = 2048,
    FONT_SIZE = 768,
    PAINTER_WORK_SPACE_SIZE = 1024,

    SECB_HPRINT_FONT = 0xF09D,  // address in SECB where HPRINT font is stored (copied to FONT_BUFFER)

    END_OF_BLOCK = 0xFE00,
    FONT_BUFFER = END_OF_BLOCK - FONT_SIZE,  // space for HPRINT 8x8 font
    DRAW_BUFFER = FONT_BUFFER - DRAW_BUFFER_SIZE,
    PAINTER_WORK_SPACE = DRAW_BUFFER - PAINTER_WORK_SPACE_SIZE,  // used by Painter object
    STACK_BOTTOM = PAINTER_WORK_SPACE,
    STACK_TOP = STACK_BOTTOM - STACK_SPACE,

    // Graphics screen.
    //
    SCREEN_WIDTH = 320,
    SCREEN_HEIGHT = 200,
    FIRST_DEFAULT_BLOCK = 56,  // the CoCo 3 boots into blocks 56..63
    FIRST_GRAPHICS_BLOCK = 48,  // this MMU block and the following three are used to store the graphics screen
    GRAPHICS_START = 0x2000,  // first address of the screen in the 64k addressable space
    GRAPHICS_END = GRAPHICS_START + SCREEN_WIDTH / 2 * SCREEN_HEIGHT,  // 2nd term ordered to avoid overflow
    BYTES_PER_PIXEL_ROW = SCREEN_WIDTH / 2,  // because 4 bits per pixel
};


#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))


// The 32k of the screen buffer is stored in MMU blocks 48..51
// and mapped at $2000..$9FFF, as done by the CoCo 3 BASIC.
// Must be consistent with GRAPHICS_START and GRAPHICS_END.
// The absolute address of the screen is $60000.
// The value to put in $FF9D to show this screen is $60000 / 8 = $C000.
//
static const byte task1MMUBlocks[8] =
{
    FIRST_DEFAULT_BLOCK,
    FIRST_GRAPHICS_BLOCK,       // GRAPHICS_START points to the start of this block.
    FIRST_GRAPHICS_BLOCK + 1,
    FIRST_GRAPHICS_BLOCK + 2,
    FIRST_GRAPHICS_BLOCK + 3,
    FIRST_DEFAULT_BLOCK + 5,
    FIRST_DEFAULT_BLOCK + 6,    // The graphics driver starts with this block
    FIRST_DEFAULT_BLOCK + 7,    // and ends in this one.
                                // These 2 blocks are always tuned in. There is where the stack lives after initDriver().
};


static word backgroundWord;

static word cc3graph_timer;


static word *
getTimerAddress(void)
{
    return &cc3graph_timer;
}


// IRQ interrupt service routine.
//
static interrupt asm void
cc3graph_irqService(void)
{
    asm
    {
_dskcon_irqService IMPORT
        ldb     $FF03
        bpl     @done               // do nothing if 63.5 us interrupt
        ldb     $FF02               // 60 Hz interrupt. Reset PIA0, port B interrupt flag.
;
        ldd     :cc3graph_timer
        addd    #1
        std     :cc3graph_timer
@done
    }
}


static Painter painter;
static Drawer drawer;


static void
initDriver(void (*postLoadFunction)(void))
{
    disableInterrupts();

    // Set up Task #1 memory block configuration, to make task1() usable.
    memcpy(0xFFA8, task1MMUBlocks, sizeof(task1MMUBlocks));

    // Move the HPRINT font from where it is in SECB to the end of the block.
    asm
    {
        ldy     #FONT_BUFFER        // destination
        ldx     #SECB_HPRINT_FONT   // source
@loop
        ldd     ,x++
        std     ,y++
        ldd     ,x++
        std     ,y++
        cmpx    #SECB_HPRINT_FONT+FONT_SIZE
        blo     @loop
    }

    backgroundWord = 0;

    // Set up driver's ISR for IRQ.
    asm
    {
        ldx     $FFF8
        lda     #$7E                    // JMP extended
        sta     ,x
        leay    cc3graph_irqService     // Y not used by CMOC-generated code when targeting Disk Basic
        sty     1,x
    }

    Painter_init(&painter, Primitive_getPixel320x16, Primitive_setPixel320x16, NULL,
                           SCREEN_WIDTH, SCREEN_HEIGHT, (byte *) PAINTER_WORK_SPACE, PAINTER_WORK_SPACE_SIZE);

    Drawer_init(&drawer, GRAPHICS_START, SCREEN_WIDTH, SCREEN_HEIGHT, 2);

    // Move the stack pointer to this driver's stack space and jump into the user program's function.
    asm
    {
        lds     #STACK_BOTTOM
        andcc   #$AF                        // enable interrupts
        jmp     [:postLoadFunction]         // allowed because U register still valid
    }
}


// MMU task selection.
// Interrupts remain enabled while Task #1 is tuned in.
//
#define task0() do { asm("clr", "$FF91"); } while (0)  /* normal mode (to run main program) */
#define task1() do { asm("ldb", "#1"); asm("stb", "$FF91"); } while (0)  /* to access graphics */
static void
showGraphicsMode(void)
{
    // Set CoCo 3 graphics mode.
    * (byte *) 0xFF90 = 0x4C;  // reset CoCo 2 compatible bit
    * (byte *) 0xFF98 = 0x80;  // graphics mode
    * (byte *) 0xFF99 = 0x3E;  // 320x200, 16 colors
    * (byte *) 0xFF9A = 0;  // border color (0..63)

    // Tell the GIME where the screen starts.
    * (unsigned *) 0xFF9D = FIRST_GRAPHICS_BLOCK << (13 - 3);  // block no * 8192 / 8
}


// Changes backgroundWord, so that the background color is remembered,
// for the use of putTextChar().
//
static void
clearGraphicsScreen(byte color)
{
    asm  // Make a word that has 'color' in all four nybbles.
    {
        ldb     :color
        lslb
        lslb
        lslb
        lslb                        ; B = color << 4
        orb     :color              ; B = color in high and low nybbles
        tfr     b,a
        std     :backgroundWord
    }
    task1();
    memset16((word *) GRAPHICS_START, backgroundWord, (GRAPHICS_END - GRAPHICS_START) / 2);
    task0();
}


static void
line(word x0, byte y0, word x1, byte y1, byte color)
{
    task1();
    Primitive_line(x0, y0, x1, y1, color, Primitive_setPixel320x16, GRAPHICS_START);
    task0();
}


static void
rectangle(word x0, byte y0, word x1, byte y1, byte color)
{
    task1();
    Primitive_rectangle(x0, y0, x1, y1, color, Primitive_setPixel320x16, GRAPHICS_START);
    task0();
}


static void
filledRectangle(word x0, byte y0, word x1, byte y1, byte color)
{
    task1();
    Primitive_filledRectangle(x0, y0, x1, y1, color, Primitive_setPixel320x16, GRAPHICS_START);
    task0();
}


static void
ellipse(word x, byte y, byte radius, byte color, word horizontalFactor, word verticalFactor)
{
    task1();
    Primitive_ellipse(x, y, radius, color, horizontalFactor, verticalFactor, Primitive_setPixel320x16, GRAPHICS_START);
    task0();
}


static byte
paint(word x, byte y, byte newColor, byte oldColor)
{
    task1();
    byte err = Painter_paint(&painter, x, y, newColor, oldColor, GRAPHICS_START);
    task0();
    return err == Painter_INVALID_ARGUMENT ? 0 : err;
}


static void
draw(const char *script)
{
    // 'scripts' is a string that is in memory that is about to be mapped out by task1(),
    // so copy it to this driver's internal buffer for DRAW scripts.
    // This is why there is a limit on the size of the script.
    //
    strncpy(DRAW_BUFFER, script, DRAW_BUFFER_SIZE);
    task1();
    Drawer_draw(&drawer, (char *) DRAW_BUFFER);
    task0();
}


// backgroundBytePtr: If null, the operation is transparent. Otherwise,
//                    *backgroundBytePtr is the (2-pixel) byte to use as the background.
//
static void
putTextChar(byte *charRowStart, char ch, byte textColor, byte *backgroundBytePtr)
{
    task1();

    const byte *charBitmap = (byte *) FONT_BUFFER + ((byte) ch - 32) * (word) 8;  // points to 8-byte array
    const byte highColorNybble = textColor << 4;
    const byte lowColorNybble  = textColor;

    for (byte y = 0; y < 8; ++y, charRowStart += BYTES_PER_PIXEL_ROW)  // for each row of bitmap to draw
    {
        byte bm = charBitmap[y];  // this byte gets shifted left at each iteration of the inner for(),
                                  // so bit 7 is always the bit currently processed
        byte *colWriter = charRowStart;  // will write 4 bytes at charRowStart
        byte screenByte;
        for (byte x = 0; x < 8; ++x, bm <<= 1)  // for each bitmap bit
        {
            if ((x & 1) == 0)  // if even-numbered pixel
            {
                // Read current pair of pixels if transparent.
                screenByte = backgroundBytePtr == NULL ? *colWriter : *backgroundBytePtr;

                if (bm & 0x80)  // change left pixel if current bitmap bit is set
                    screenByte = screenByte & 0x0F | highColorNybble;
            }
            else  // odd-numbered pixel
            {
                if (bm & 0x80)  // change right pixel if current bitmap bit is set
                    screenByte = screenByte & 0xF0 | lowColorNybble;
                *colWriter++ = screenByte;
            }
        }
    }

    task0();
}


static void
printTextAtPixelRow(byte textColumn, byte pixelRow, const char *text, byte textColor, byte backgroundColor)
{
    if (textColumn >= SCREEN_WIDTH / 8 || pixelRow > SCREEN_HEIGHT - 8 || *text == '\0')
        return;
#if 0
    byte *charRowStart = (byte *) GRAPHICS_START + (word) pixelRow * BYTES_PER_PIXEL_ROW + (textColumn << 2);
#else
    byte *charRowStart;
    asm  // This asm section avoids letting the compiler call MUL16 to do (word) pixelRow * BYTES_PER_PIXEL_ROW.
    {
        // Multiply pixelRow by BYTES_PER_PIXEL_ROW, which is (4 + 1) * 32, i.e., (4 + 1) << 5.
        ldb     :pixelRow       // B = 0..199
        clra
        lslb
        rola
        lslb
        rola                    // D = pixelRow * 4
        addb    :pixelRow
        adca    #0              // D = pixelRow * (4 + 1)
        lslb
        rola
        lslb
        rola
        lslb
        rola
        lslb
        rola
        lslb
        rola                    // D = pixelRow * (4 + 1) << 5 = pixelRow * BYTES_PER_PIXEL_ROW
        tfr     d,x

        ldb     :textColumn     // 0..39
        lslb
        lslb                    // B = textColumn << 2
        abx
        leax    GRAPHICS_START,x
        stx     :charRowStart
    }
#endif

    byte backgroundByte, *backgroundBytePtr;
    if (backgroundColor > 15)  // if transparent
        backgroundBytePtr = NULL;
    else
    {
        backgroundByte = backgroundColor | (backgroundColor << 4);
        backgroundBytePtr = &backgroundByte;
    }

    do
    {
        putTextChar(charRowStart, *text, textColor, backgroundBytePtr);
        charRowStart += 4;  // 4 bytes per text column, i.e., 8 pixels
        ++text;
    } while (*text != '\0' && textColumn < SCREEN_WIDTH / 8);
}


static void
printText(byte textColumn, byte textRow, const char *text, byte textColor, byte backgroundColor)
{
    if (textRow >= SCREEN_HEIGHT / 8)
        return;
    printTextAtPixelRow(textColumn, textRow << 3, text, textColor, backgroundColor);
}


GraphicsDriver theGraphicsDriver =
{
    initDriver,
    clearGraphicsScreen,
    showGraphicsMode,
    line,
    rectangle,
    filledRectangle,
    ellipse,
    paint,
    draw,
    printText,
    printTextAtPixelRow,
    getTimerAddress,
};


int
main()
{
    return (int) &theGraphicsDriver;
}
