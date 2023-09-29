/*  graph.c - Test demo of this library.
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "Drawer.h"
#include "Painter.h"
#include "semigr.h"


enum
{
    workSpaceCapacity = 24,        // Scratch space for Painter_paint().
};


static byte *screenBuffer;


struct VDGModeDesc
{
    byte screenWidthInPixels;
    byte screenHeightInPixels;
    byte pixelsPerByte;
    byte samVRegisterValue;
    byte num512BytePages;
};


// These modes are described on page 70 of Chris Lomont's
// "Color Computer 1/2/3 Hardware Programming", v0.82.
//
struct VDGModeDesc vdgModeDescArray[3] =
{
    {  64, 64, 4, 1, 2 },
    { 128, 64, 8, 1, 2 },
    { 128, 64, 4, 2, 4 },
};


static BOOL
drawDemo()
{
    byte pmodeNo;
    byte vdgMode;  // index into vdgModeDescArray[]
    printf("PMODE (0-4, V0, V1, V2)? ");
    const char *response = readline();
    if (*response == 'V')
    {
        vdgMode = (byte) atoi(response + 1);  // V0..2 will be 0..2
        pmodeNo = 255;
    }
    else
    {
        vdgMode = 255;
        pmodeNo = (byte) atoi(response);
    }
    if (pmodeNo == 255 && vdgMode > 2 || vdgMode == 255 && pmodeNo > 4)
    {
        printf("INVALID MODE (%u, %u).\n", pmodeNo, vdgMode);
        return FALSE;
    }

    byte colorset = 1;
    printf("COLOR SET (0, 1)? ");
    response = readline();
    if (*response == '0')
        colorset = 0;

    Drawer drawer;
    
    if (vdgMode == 255)  // if PMODE mode chosen
    {
        if (!Drawer_initForPMODE(&drawer, screenBuffer, pmodeNo))
        {
            printf("FAILED TO INIT DRAWER.\n");
            return FALSE;
        }

        if (!pmode(pmodeNo, screenBuffer))
        {
            printf("PMODE: INVALID PARAMETERS.\n");
            return FALSE;
        }
        pcls(0);  // black in PMODE 4, white in PMODE 3
        screen(1, colorset);
    }
    else
    {
        if (!Drawer_init(&drawer, screenBuffer,
                         vdgModeDescArray[vdgMode].screenWidthInPixels,
                         vdgModeDescArray[vdgMode].screenHeightInPixels,
                         vdgModeDescArray[vdgMode].pixelsPerByte))
        {
            printf("FAILED TO INIT DRAWER.\n");
            return FALSE;
        }

        // Equivalent of PMODE:
        byte *pia1bData = (byte *) 0xff22;
        *pia1bData = *pia1bData & 7 | (vdgMode << 4);
        
        // PCLS:
        memset(screenBuffer, 0, vdgModeDescArray[vdgMode].num512BytePages * 512);
        
        // SCREEN:
        *pia1bData |= 0x80 | (vdgMode << 4) | (colorset << 3);
        showGraphicsAddress(vdgModeDescArray[vdgMode].samVRegisterValue,
                            (byte) ((word) screenBuffer / 512));
    }

    const char *script =
            "D15"
            "R10"
            "BU8"
            "R5"
            "D0"
            "R30"
            "D12"
            "L7"
            "NU3"
            "N;L3"  // ';' allowed between N and draw command
            "NR20"
            "N;D3"
            "BD20"
            "D0"    // isolated pixel
            "BR6"
            "E9"    // begin losange    
            "F9"
            "G9"
            "H9"    // end losange
            "M20,181"       // draw diag. line to near bottom left
            "B;M30,45;"     // move to near top left
            "R50;"
            "M-4,-15"      // rel. move up and a little to the left
            // Test black (C0), assuming PMODE 4:
            "BM-5,0"
            "L9BD1"  // draw 3 consecutive horiz. white lines
            "R9BD1"
            "L9BU1"
            "C0R9"   // "erase" middle line by drawing over it in black
            "C1BD3"  // back to white, go down (in PMODE 3: switches to cyan)
            "L9"     // draw white line below the 2 remaining white lines
            // Test out of bounds move:
            "BD2;M400,200"  // actually goes to bottom right
            "M-400,-500"   // actually goes to top left
            ;
    const char *errorPos = Drawer_draw(&drawer, script);
    if (errorPos)
    {
        screen(0, 0);
        printf("ERROR: STOPPED AT INVALID COMMAND '%c' AT OFFSET %u\n",
               *errorPos, errorPos - script);
        return 1;
    }

    // Demo some lines, circles and ellipses if PMODE 4.
    if (pmodeNo == 4)
    {
        // Draw a radial pattern at the left of the screen.
        for (byte y = 55; y <= 135; y += 20)
            Primitive_line(15, y, 41, 190 - y, 1, Primitive_setPixelPmode4, (void *) screenBuffer);
    
        //setTimer(0);

        for (byte radius = 5; radius <= 95; radius += 10)
            Primitive_circle(128, 96, radius, 1, Primitive_setPixelPmode4, (void *) screenBuffer);

        //word elapsed = getTimer();
        //printf("ELAPSED: %u", elapsed);

        Primitive_ellipse(128, 96, 23, 1, 256 * 4, 256, Primitive_setPixelPmode4, (void *) screenBuffer);
        Primitive_ellipse(128, 96, 23, 1, 256, 256 * 4, Primitive_setPixelPmode4, (void *) screenBuffer);

        Primitive_ellipse(212, 45, 43, 1, 256 / 2, 256, Primitive_setPixelPmode4, (void *) screenBuffer);
        Primitive_ellipse(212, 45, 43, 1, 256, 256 / 2, Primitive_setPixelPmode4, (void *) screenBuffer);
        Primitive_circle( 212, 45, 43, 1, Primitive_setPixelPmode4, (void *) screenBuffer);

        Primitive_ellipse(212, 145, 43, 1, 256 / 4, 256, Primitive_setPixelPmode4, (void *) screenBuffer);
        Primitive_ellipse(212, 145, 43, 1, 256, 256 / 4, Primitive_setPixelPmode4, (void *) screenBuffer);
        Primitive_circle( 212, 145, 43, 1, Primitive_setPixelPmode4, (void *) screenBuffer);
    }
    
    return TRUE;
}


static BOOL paintInterrupted;


// Let the painting continue if the user has not pressed the Break key.
//
static BOOL
paintDemoContinuationFunction(void *cookie)
{
    byte key = inkey();
    //printf("KEY=%u\n", key);
    if (key != 3)
        return TRUE;
    paintInterrupted = TRUE;
    return FALSE;
}


static BOOL
paintDemo()
{
    Painter painter;
    
    byte workSpace[workSpaceCapacity];
    memset(workSpace, 0xFF, sizeof(workSpace));  // fill to later see how much was actually used

    if (!Painter_init(&painter,
                        Primitive_getPixelPmode4, Primitive_setPixelPmode4, paintDemoContinuationFunction,
                        256, 192, workSpace, workSpaceCapacity))
    {
        printf("FAILED TO INIT PAINTER.\n");
        return FALSE;
    }
    
    if (!pmode(4, screenBuffer))
    {
        printf("PMODE: INVALID PARAMETERS.\n");
        return FALSE;
    }
    pcls(0);  // black in PMODE 4, white in PMODE 3
    screen(1, 1);

    Drawer drawer;
    if (!Drawer_initForPMODE(&drawer, screenBuffer, 4))
    {
        printf("FAILED TO INIT DRAWER.\n");
        return FALSE;
    }

    
    const char *script = "BM128,96;U20L10D40L50H50E50R10G40R25H5E15R5F15E20R30F25D10G15D10E30U20H50;"
                         "BM128,96;D15F5G5D15G25H10G20F10R80E15F10R10E15H5U15E10H20G20R5D5L20H10G5H10E30R50F40";
    const char *errorPos = Drawer_draw(&drawer, script);
    if (errorPos)
    {
        screen(0, 0);
        printf("ERROR: STOPPED AT INVALID COMMAND '%c' AT OFFSET %u\n",
               *errorPos, errorPos - script);
        return 1;
    }

    BOOL success = FALSE;
    word startTime = getTimer();
    do
    {
        paintInterrupted = FALSE;

        // Paint white over black.
        byte err = Painter_paint(&painter, 240, 80, 1, 0, (void *) screenBuffer);
        if (err != Painter_NO_ERROR)
        {
            printf("1ST PAINT FAILED: ERROR #%u.\n", err);
            break;
        }
        if (paintInterrupted)
        {
            printf("1ST PAINT INTERRUPTED.\n");
            break;
        }

        paintInterrupted = FALSE;

        err = Painter_paint(&painter, 27, 177, 1, 0, (void *) screenBuffer);
        if (err != Painter_NO_ERROR)
        {
            printf("2ND PAINT FAILED: ERROR #%u.\n", err);
            break;
        }
        if (paintInterrupted)
        {
            printf("2ND PAINT INTERRUPTED.\n");
            break;
        }

        success = TRUE;
    } while (FALSE);

    printf("PAINTING TOOK %u MS\n", (getTimer() - startTime) * 50 / 3);
    return success;
}


static byte
get64x32TargetByteAndMask(word x, byte y, byte *textScreen, byte **targetBytePtr)
{
    *targetBytePtr = textScreen + ((word) (y >> 1) << 5) + (x >> 1);
    return ((x & 1) ? 0x01 : 0x02) << ((y & 1) ? 0 : 2);
}


// x: 0..63.
// y: 0..31.
// color: 0..7.
// cookie: Text screen address.
//
static void
setPixel64x32(word x, byte y, byte color, void *cookie)
{
    byte *targetByte;
    byte mask = get64x32TargetByteAndMask(x, y, (byte *) cookie, &targetByte);
    *targetByte = ((*targetByte | mask) & 0x8F) | (color << 4);
}


static byte
getPixel64x32(word x, byte y, void *cookie)
{
    byte *targetByte;
    byte mask = get64x32TargetByteAndMask(x, y, (byte *) cookie, &targetByte);
    byte b = *targetByte;
    if (b & mask)  // if pixel set
        return (b & 0x70) >> 4;  // convert bits 4..6 to color code in 0..7
    return 0;  // pixel not set: return color code for black
}


BOOL
paint64x32Demo(void)
{
    Painter painter;
    
    byte workSpace[workSpaceCapacity];

    if (!Painter_init(&painter, getPixel64x32, setPixel64x32, NULL, 64, 32, workSpace, workSpaceCapacity))
    {
        printf("FAILED TO INIT PAINTER.\n");
        return FALSE;
    }

    cls(0);  // clear screen to black
    screen(0, 0);

    void *textScreen = (void *) 0x0400;

    // A single color is used for drawing the circles and for painting,
    // because using a second non-black color would cause some pixels
    // to "bleed" onto others, because two pixels residing in the same byte
    // must be of a single color.
    //
    byte color = 3;  // red

    // Draw circles.
    //
    Primitive_circle(31, 15, 5, color, setPixel64x32, textScreen);
    Primitive_circle(31, 15, 13, color, setPixel64x32, textScreen);

    sleep(1);

    // Paint the interior of the inner circle.
    //
    byte err = Painter_paint(&painter, 31, 15, color, 0, textScreen);
    if (err != Painter_NO_ERROR)
    {
        printf("1ST PAINT FAILED: ERROR #%u.\n", err);
        return FALSE;
    }

    // Paint the exterior of the outer circle.
    //
    err = Painter_paint(&painter, 0, 0, color, 0, textScreen);
    if (err != Painter_NO_ERROR)
    {
        printf("2ND PAINT FAILED: ERROR #%u.\n", err);
        return FALSE;
    }

    return TRUE;
}


const char *presentation =
"THIS PROGRAM DRAWS A TRIANGLE, THEN ANIMATES A BOUNCING BALL. "
"AS THE BALL TRAVERSES THE EDGES OF THE TRIANGLES, ARTIFACTS OF THE "
"DOT'S COLOR ARE LEFT BEHIND. THIS ILLUSTRATES THAT TWO SET PIXELS "
"THAT ARE PART OF THE SAME BYTE SHARE A SINGLE COLOR.\n";


void
advance(byte *value, char *delta, byte maxValue)
{
    for (;;)
    {
        int newValue = (int) *value + *delta;
        if (newValue >= 0 && (byte) newValue < maxValue)
        {
            *value = (byte) newValue;
            return;
        }
        *delta = - *delta;
    }
}


typedef struct LineDrawingCookie
{
    SetPixelFunction setPixelFunction;
    byte *screenBuffer;
} LineDrawingCookie;


static void
lineSetPixel(word x, byte y, byte color, void *cookie)
{
    const LineDrawingCookie *ldc = (LineDrawingCookie *) cookie;
    //printf("LSP(%u,%u,%u,[%p,%p])", x, y, color, ldc->setPixelFunction, ldc->screenBuffer);
    (*ldc->setPixelFunction)(ldc->screenBuffer, (byte) x, y, color);
}


// Prints text with word wrap.
//
static void
printText(const char *text)
{
    byte charsPerLine = 31;
    for (byte horizPos = 0; *text != '\0'; )
    {
        const char *wordEnd = strchr(text, ' ');
        BOOL spaceFound = TRUE;
        if (!wordEnd)
        {
            spaceFound = FALSE;
            wordEnd = text + strlen(text);
        }
        byte wordLen = (byte) (wordEnd - text);
        if (horizPos + wordLen > charsPerLine)
        {
            putchar('\n');
            horizPos = 0;
        }
        putstr(text, wordLen);
        horizPos += wordLen;
        if (spaceFound && horizPos < charsPerLine)
        {
            putchar(' ');
            ++horizPos;
        }

        for (text = wordEnd; *text == ' '; ++text)  // skip source spaces
            ;
    }
}


void
semiGraphicsDemo(void)
{
    cls(255);
    printText(presentation);

    for (;;)
    {
        byte sgModeNo;

        printf("\nMODE?\n[8] SG8  [2] SG12  [4] SG24\n[BREAK] BACK\n> ");
        for (;;)
        {
            byte key = waitkey(TRUE);
            if (key == '8')
            {
                sgModeNo = 8;
                break;
            }
            if (key == '2' || key == '4')
            {
                sgModeNo = (key - '0') * 6;
                break;
            }
            if (key == 3)
            {
                putchar('\n');
                return;
            }
        }
        printf("SG%u\n", sgModeNo);

        // Get start of graphics RAM from Color Basic variable.
        byte *screenBuffer = (byte *) (((word) * (byte *) 0x00BC) << 8);

        byte screenWidthInPixels = 64, screenHeightInPixels;
        word screenBufferSize;  // bytes
        SetPixelFunction setPixelFunction;
        GetPixelFunction getPixelFunction;
        switch (sgModeNo)
        {
        case 8:
            screenHeightInPixels = 64;
            screenBufferSize = 2048;
            setPixelFunction = setPixelSG8;
            getPixelFunction = getPixelSG8;
            break;
        case 12:
            screenHeightInPixels = 96;
            screenBufferSize = 3072;
            setPixelFunction = setPixelSG12;
            getPixelFunction = getPixelSG12;
            break;
        case 24:
            screenHeightInPixels = 192;
            screenBufferSize = 6144;
            setPixelFunction = setPixelSG24;
            getPixelFunction = getPixelSG24;
            break;
        default:
            printf("INVALID MODE %u\n", sgModeNo);
            exit(1);
        }

        // Clear the screen to black. This must set bit 7 of all bytes.
        memset(screenBuffer, 0b10000000, screenBufferSize);

        // Wait for the VSync interrupt, to change the screen mode
        // during the vertical blanking interval.
        //
        asm { sync }

        if (!sgMode(sgModeNo, screenBuffer))
        {
            printf("FAILED TO SET UP SEMIGRAPHICS\nMODE %u AT %p.\n", sgModeNo, screenBuffer);
            exit(1);
        }

        // Draw a triangle.
        //
        LineDrawingCookie ldc = { setPixelFunction, screenBuffer };
        byte leftX = 0, midX = screenWidthInPixels / 2, rightX = screenWidthInPixels - 1;
        byte topY = 0, bottomY = screenHeightInPixels - 1;
        byte lineColor = 4;
        Primitive_line(midX,   topY,    leftX,  bottomY, lineColor, lineSetPixel, &ldc);
        Primitive_line(leftX,  bottomY, rightX, bottomY, lineColor, lineSetPixel, &ldc);
        Primitive_line(rightX, bottomY, midX,   topY,    lineColor, lineSetPixel, &ldc);

        // Show all non-black colors. Test getPixelFunction.
        byte midY = screenHeightInPixels / 2;
        for (byte color = 1; color <= 8; ++color)
        {
            byte x = 32 - 8 + (color - 1) * 2;
            byte lineTopY = midY - sgModeNo;
            Primitive_line(x, lineTopY, x, midY + sgModeNo, color, lineSetPixel, &ldc);

            byte colorRead = (*getPixelFunction)(screenBuffer, x, lineTopY);
            if (colorRead != color)
            {
                sgMode(0, 0x0400);  // back to 32x16 at usual address
                printf("ERROR: EXPECTED %u AT (%u,%u), GOT %u.\n", color, x, lineTopY, colorRead);
                exit(1);
            }
        }
        byte colorRead = (*getPixelFunction)(screenBuffer, 0, 0);
        if (colorRead != 0)
        {
            sgMode(0, 0x0400);  // back to 32x16 at usual address
            printf("ERROR: EXPECTED 0 AT (0,0), GOT %u.\n", colorRead);
            exit(1);
        }

        // Animate a dot.
        //
        byte x = 22, y = screenHeightInPixels - 32;
        char dx = 1, dy = 1;
        byte dotColor = 2;
        
        while (!inkey())
        {
            byte origX = x, origY = y;
            advance(&x, &dx, screenWidthInPixels);
            advance(&y, &dy, screenHeightInPixels);
            
            // Wait for the VSync interrupt, then change the screen buffer
            // during the vertical blanking interval.
            asm { sync }
            (*setPixelFunction)(screenBuffer, origX, origY, 0);
            (*setPixelFunction)(screenBuffer, x, y, dotColor);
        }

        sgMode(0, 0x0400);  // back to 32x16 at usual address
    }
}


int
main()
{
    initCoCoSupport();

    rgb();
    width(32);

    // Get start of graphics RAM from Color Basic variable.
    screenBuffer = (byte *) (((word) * (byte *) 0x00BC) << 8);

    for (BOOL keepOn = TRUE; keepOn; )
    {
        cls(255);
        printf("[D] DRAW DEMO\n"
               "[P] PMODE 4 PAINT\n"
               "[6] 64X32 PAINT\n"
               "[S] SEMI-GRAPHICS (COCO 1 & 2)\n"
               "[T] 32X16 TEXT COLOR SET\n"
               "[BREAK] QUIT\n"
               "? ");
        char demo = (char) toupper(waitkey(TRUE));
        printf("%c\n", isalnum(demo) ? demo : ' ');
        delay(2);

        switch (demo)
        {
            case 'D':
                drawDemo();
                waitkey(FALSE);
                break;
            case 'P':
                paintDemo();
                waitkey(FALSE);
                break;
            case '6':
                paint64x32Demo();
                waitkey(FALSE);
                break;
            case 'S':
                if (isCoCo3)
                {
                    printf("THIS PROGRAM CANNOT RUN ON\nA COCO 3.\n");
                    waitkey(FALSE);
                }
                else
                    semiGraphicsDemo();
                break;
            case 'T':
                // Demo SCREEN 0,1.
                // Note that printf() uses Basic's CHROUT, which restores SCREEN 0,0. 
                // Therefore, we do SCREEN 0,1 after the printf(), then sleep to show
                // the orange screen, then do SCREEN 0,0 to restore the green mode.
                cls(255);
                for (byte i = 0; i < 3; ++i)
                {
                    printf("ORANGE SCREEN...\n");
                    screen(0, 1);
                    sleep(1);
                    printf("GREEN SCREEN...\n");
                    screen(0, 0);
                    sleep(1);
                }
                printf("PRESS A KEY...\n");
                waitkey(FALSE);
                break;
            case 3:
                keepOn = FALSE;
                break;
        }
    }
    
    cls(255);
    printf("THE END.\n\n");
    return 0;
}
