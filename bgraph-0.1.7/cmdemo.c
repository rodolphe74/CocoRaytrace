/*  catmull.c - Demo of a Catmull-Rom splines for the CoCo.
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.

    Requires a 32K CoCo.
    Makes calls to Extended Color Basic.
*/

#include "catmull-rom.h"
#include "clipping.h"
#include "Primitive.h"

#include <assert.h>


enum
{
    NUM_RANDOM_PATH_POINTS = 8,
    MAX_NUM_USER_PATH_POINTS = 20,
    SCREEN_WIDTH = 256,
    SCREEN_HEIGHT = 192,
    CLEAR_KEY = 12,
    DRAW_KEY = ' ',
    DEL_POINT_KEY = 8,
    MARGIN = 10,
    NUM_CURSOR_PIXELS = 5,
    MC6847_CAPITAL_X = 'X',
    MC6847_SPACE = 0x60,
    MAX_BANGS = 2,
};


const ShortClipRect screenRect = { { 0, 0 }, { SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 } };
byte *screenBuffer;

BOOL autoMode = TRUE;
BOOL pointsSorted = TRUE;
BOOL openPath;
BOOL soundEnabled = TRUE;
BOOL scalePlayedBackwards;
BOOL useHighSpeed;
BOOL reuseSamePoints;
BOOL tangentsDrawn;
BOOL coco3Graphics;

// Must be called between tuneInGraphicsBuffer() and tuneOutGraphicsBuffer().
Primitive_setPixelFuncPtr setPixelFuncPtr;
Primitive_getPixelFuncPtr getPixelFuncPtr;

byte bgColor;
byte ctrlPtColor;
byte lineColor;
byte tangentColor;
BOOL quitRequested;


void
sleepTicks(size_t numTicks)
{
    for (size_t limit = getTimer() + numTicks; (int) (getTimer() - limit) <= 0; )
        ;
}


// Sleeps until the given number of ticks has elapsed or until Enter or Break
// has been pressed.
// Returns FALSE if Break was pressed.
BOOL
sleepAndCheckKeys(size_t numTicks)
{
    for (size_t limit = getTimer() + numTicks; (int) (getTimer() - limit) <= 0; )
    {
        byte key = inkey();
        if (key == 3)
            return FALSE;
        if (key == 13)
            return TRUE;
    }
    return TRUE;
}


BOOL
interpretCommand(byte key)
{
    BOOL beep = TRUE;
    switch (key)
    {
        case 3:  // Break
            quitRequested = TRUE;
            return FALSE;
        case 'M':  // mute/unmute
            soundEnabled = !soundEnabled;
            break;
        case 'A':
            autoMode = !autoMode;
            break;
        case 'R':
            reuseSamePoints = TRUE;
            break;
        case 'S':
            pointsSorted = !pointsSorted;
            break;
        case 'H':
            useHighSpeed = !useHighSpeed;
            setHighSpeed(useHighSpeed);
            break;
        case 'P':
            openPath = !openPath;
            break;
        case 'T':
            tangentsDrawn = !tangentsDrawn;
            break;
        default:
            reuseSamePoints = FALSE;
            beep = FALSE;
    }
    if (beep)
    {
        sound(160, 1);  // indicate that a change has been registered
        sleepTicks(20);
    }
    return TRUE;
}


// Use the 32k before the default 64k to contain the CoCo 3 graphics screen buffer.
//
static const byte task1MMUBlocks[8] = { 56, 57, 58, 59, 52, 53, 54, 55 };


// RGB approximation of the 16 colors specified by the Berkeley Logo User Manual
// for the SETPENCOLOR command:
//       0  black        1  blue         2  green        3  cyan
//       4  red          5  magenta      6  yellow       7 white
//       8  brown        9  tan         10  forest      11  aqua
//      12  salmon      13  purple      14  orange      15  grey
//
static const byte ucbLogoPaletteValues[16] =
{
     0,  9, 18, 27,
    36, 45, 54, 63,
    34, 53, 20, 29,
    60, 40, 38, 56,
};


// MMU task selection.
//
#define task0() asm("clr", "$FF91")  /* normal mode (to run main program) */
#define task1() do { asm("ldb", "#1"); asm("stb", "$FF91"); } while (0)  /* to access graphics */


// tuneOutGraphicsBuffer() must be called when finished accessing the graphics buffer.
//
static void
tuneInGraphicsBuffer(void)
{
    if (coco3Graphics)
    {
        disableInterrupts();
        task1();
    }
}


static void
tuneOutGraphicsBuffer(void)
{
    if (coco3Graphics)
    {
        task0();
        enableInterrupts();
    }
}


// Does not change the screen mode.
//
void
clearGraphicsScreen(void)
{
    if (coco3Graphics)
    {
        tuneInGraphicsBuffer();

        // Clear the 24k buffer at 'screenBuffer' (i.e., 256x192 pixels, 2 pixels per byte).
        // Loop unrolling gives better performance, and we have the memory for it.
        void *bufferEnd;
        asm
        {
            ldx     :screenBuffer
            leax    SCREEN_WIDTH*SCREEN_HEIGHT/2,x
            stx     :bufferEnd
            clra
            clrb
            ldx     :screenBuffer
@loop
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            std     ,x++
            cmpx    :bufferEnd
            blo     @loop
        }

        tuneOutGraphicsBuffer();
    }
    else
    {
        memset16(screenBuffer, 0x0000, 1536 * 4 / 2);
    }
}


// When using CoCo 3 graphics, tunes out the Basic interpreter.
//
static void
showGraphicsMode(void)
{
    * (byte *) 0x41F = coco3Graphics;
    if (coco3Graphics)
    {
        asm { sync }  // wait until v-sync to switch to graphics mode

        // Set CoCo 3 graphics mode.
        * (byte *) 0xFF90 = 0x4C;  // reset CoCo 2 compatible bit
        * (byte *) 0xFF98 = 0x80;  // graphics mode
        * (byte *) 0xFF99 = 0b00011010;  // 256x192, 16 colors (24k buffer)
        * (byte *) 0xFF9A = 1;  // border color (0..63)

        * (unsigned *) 0xFF9D = 0xD000;  // 0x68000 >> 3, i.e., address of block 52 divided by 8

        // Initialize the palette. Assumes RGB.
        memcpy((void *) 0xFFB0, ucbLogoPaletteValues, sizeof(ucbLogoPaletteValues));
    }
    else
    {
        // PMODE 4:
        byte *pia1bData = (byte *) 0xff22;
        byte b = *pia1bData & 7 | ((4 /*pmode*/ + 3) << 4);

        // SCREEN 1,1:
        asm { sync }  // wait until v-sync to switch to graphics mode
        *pia1bData = b | 0x80 | 0x08;  // graphics mode, color set #1
        showGraphicsAddress(6 /*PMODE 4*/, (byte) ((word) screenBuffer / 512));
    }
}


byte *
getPModeGraphicsBuffer(void)
{
    return (byte *) * (byte *) 0x00BC << 8;
}


// Applies the value of coco3Graphics.
// Does not change the screen mode. See showGraphicsMode() and showTextMode().
//
void
initGraphics(void)
{
    if (coco3Graphics)
    {
        memcpy(0xFFA8, task1MMUBlocks, 8);  // configure memory blocks of MMU task #1 re: task1()
        screenBuffer = 0x8000;
        setPixelFuncPtr = Primitive_setPixel256x16;
        getPixelFuncPtr = Primitive_getPixel256x16;
        ctrlPtColor = 3;  // see ucbLogoPaletteValues[]
        lineColor = 6;
        tangentColor = 4;
    }
    else  // Set up PMODE 4 graphics mode.
    {
        screenBuffer = getPModeGraphicsBuffer();  // use Basic's PMODE buffer
        setPixelFuncPtr = Primitive_setPixelPmode4;
        getPixelFuncPtr = Primitive_getPixelPmode4;
        ctrlPtColor = 1;  // white with NTSC artifacts
        lineColor = 1;
        tangentColor = 1;
    }
}


// When using CoCo 3 graphics, tunes the Basic interpreter back in.
//
void
showTextMode(void)
{
    asm { sync }  // wait until v-sync to switch to text mode
    rgb();
    palette(12, 63);  // text foreground color
    palette(13, 0);   // text background color
    width(32);
    screen(0, 0);
}


// cookie: Color of the line (0..255), converted to a pointer.
//
BOOL
drawLine(short x0, short y0, short x1, short y1, void *cookie)
{
    if (!interpretCommand(inkey()))
        return FALSE;  // user asked to quit

    //printf("DRAWLINE(%4d,%4d,%4d,%4d)\n", x0, y0, x1, y1);
    if (cohenSutherlandLineClip(&x0, &y0, &x1, &y1, &screenRect))  // if line inside screen
    {
        tuneInGraphicsBuffer();
        Primitive_line((word) x0, (byte) y0, (word) x1, (byte) y1, (byte) cookie, setPixelFuncPtr, screenBuffer);
        tuneOutGraphicsBuffer();
    }
    return TRUE;  // continue drawing
}


// Values of SOUND command's first argument that correspond to a musical scale.
//
const byte scale[] =
{
      5,    // F
     32,
     58,
     69,    // B flat
     89,    // middle C
    108,
    125,
    133,
};


static void
drawControlPoint(const ShortPoint *pt, BOOL on)
{
    tuneInGraphicsBuffer();
    (*setPixelFuncPtr)((word) pt->x, (byte) pt->y, on ? ctrlPtColor : bgColor, screenBuffer);
    tuneOutGraphicsBuffer();
}


static void
drawControlPoints(const ShortPoint points[], byte numPoints)
{
    for (byte i = 0; i < numPoints; ++i)
        drawControlPoint(&points[i], TRUE);
}


static BOOL
onPieceEnd(size_t pointIndex, void *cookie)
{
    if (soundEnabled)
    {
        if (openPath)
            ++pointIndex;  // 0..6 -> 1..7
        sound(scale[scalePlayedBackwards ? 7u - pointIndex : pointIndex], 1);
    }
    return TRUE;
}


// Coordinates are assumed to be (0..255, 0..191).
//
static unsigned long
computeSquareDistance(const ShortPoint *a, const ShortPoint *b)
{
    unsigned short dx = (unsigned short) abs(b->x - a->x);  // max 255
    unsigned short dy = (unsigned short) abs(b->y - a->y);  // max 191
    return (unsigned long) (dx * dx) + (dy * dy);
}


static byte
findClosestPoint(const ShortPoint *refPoint, const ShortPoint points[], byte begin, byte end)
{
    assert(end - begin >= 2);
    unsigned long shortestSquareDisance = 0xFFFFFFFF;
    byte closestPointIndex = begin;
    for (byte i = begin; i != end; ++i)
    {
        unsigned long sqDist = computeSquareDistance(refPoint, &points[i]);
        if (sqDist < shortestSquareDisance)
        {
            shortestSquareDisance = sqDist;
            closestPointIndex = i;
        }
    }
    return closestPointIndex;
}


static void
sortPoints(ShortPoint points[], byte numPoints)
{
    // For each point P, replace its successor by the point closest to P.
    for (byte i = 2; i < numPoints; ++i)
    {
        const ShortPoint *refPoint = &points[i - 2];
        byte closestPointIndex = findClosestPoint(refPoint, points, i - 1, numPoints);

        if (closestPointIndex != i - 1)
        {
            // Swap points at i - 1 and closestPointIndex.
            ShortPoint temp = points[i - 1];
            points[i - 1] = points[closestPointIndex];
            points[closestPointIndex] = temp;
        }
    }
}


#if 0
static void
dumpPoints(const ShortPoint points[], byte numPoints)
{
    printf("\n");
    for (byte i = 0; i < numPoints; ++i)
    {
        printf("(%d,%d) ", points[i].x, points[i].y);
        if (i > 0)
        {
            unsigned long sd = computeSquareDistance(&points[i - 1], &points[i]);
            printf("%lu ", sd);
        }
    }
    printf("\n");
}
#endif


void
clearTextScreen(void)
{
    cls(isCoCo3 ? 0 : 255);
}


const char *helpLines[] =
{
    "",
    "MENU DURING THE DEMO:",
    "",
    "brk  SHOW THIS MENU",
    "m    MUTE/UNMUTE SOUND",
    "s    TOGGLE SORT MODE",
    "p    TOGGLE OPEN/CLOSED PATH",
    "t    TOGGLE TANGENTS",
    "a    TOGGLE AUTO MODE",
    "h    TOGGLE HIGH SPEED",
    "",
    NULL,
};


// Goes to text mode.
//
void
showDemoHelp(void)
{
    clearTextScreen();
    showTextMode();

    byte maxLen = 0;  // longest length in helpLines[]
    for (byte i = 0; helpLines[i]; ++i)
    {
        byte len = (byte) strlen(helpLines[i]);
        if (len > maxLen)
            maxLen = len;
    }
    byte leftCol = (32 - 2 - maxLen) / 2;
    for (byte i = 0; helpLines[i]; ++i)
    {
        locate(leftCol, 1 + i);
        byte len = (byte) strlen(helpLines[i]);
        putchar(' ');
        putstr(helpLines[i], len);
        for (byte j = maxLen - len + 1; j--; )
            putchar(' ');
    }
}


// Assumes that the text mode is on.
// Goes to graphics mode if the user does not quit.
//
BOOL
continueToGraphics(void)
{
    locate(3, 13);
    printf("    enter TO CONTINUE     ");
    locate(3, 14);
    printf("    break TO SHOW MENU    ");
    byte key;
    do
    {
        key = waitkey(FALSE);
        if (key == 3)
            return TRUE;
    } while (key != 13);    

    quitRequested = FALSE;
    showGraphicsMode();
    return FALSE;
}


static void
add(ShortPoint *dest, const ShortPoint *a, const ShortPoint *b)
{
    dest->x = a->x + b->x;
    dest->y = a->y + b->y;
}


static void
sub(ShortPoint *dest, const ShortPoint *a, const ShortPoint *b)
{
    dest->x = a->x - b->x;
    dest->y = a->y - b->y;
}


// Approximation of the norm of a vector.
// p: Fields x and y must be in -255..255.
// Computes the norm of half the vector, then doubles the result.
// Does not do 32-bit arithmetic.
// Returns 0..510.
//
static word
normOfByteCouple(ShortPoint *p)
{
    #if defined(_CMOC_VERSION_) && _CMOC_VERSION_ >= 1080  /* if CMOC >= 0.1.80 */
    return sqrt32((unsigned long) ((long) p->x * p->x + (long) p->y * p->y));
    #else
    // Avoid 32-bit arithmetic by computing the length of half the segment, then doubling the result.
    byte halfX = (byte) abs(p->x) / 2, halfY = (byte) abs(p->y) / 2;
    return 2 * (word) sqrt16((word) halfX * halfX + (word) halfY * halfY);
    #endif
}


// a, b: Must be in (0..255, 0..255).
// newLength: Must be 0..15.
//
static void
subAndSetLength(ShortPoint *dest, byte newLength, const ShortPoint *a, const ShortPoint *b)
{
    sub(dest, a, b);  // absolute value of *dest will be (255, 255), i.e., fields x and y each need 9 bits including sign bit
    short curLength = (short) normOfByteCouple(dest);  // max will be sqrt(255**2 * 255**2) = 360.6 +/- a few units
    dest->x = dest->x * (short) newLength / curLength;  // the multiplication is 9 bits times 4 bits: no overflow
    dest->y = dest->y * (short) newLength / curLength;
}


static void
drawTangent(const ShortPoint *points, byte numPoints, byte pointIndex)
{
    const ShortPoint *precPoint = &points[(pointIndex == 0 ? numPoints : pointIndex) - 1];
    const ShortPoint *nextPoint = &points[pointIndex == numPoints - 1 ? 0 : pointIndex + 1];
    ShortPoint fromPrecToNext;
    subAndSetLength(&fromPrecToNext, 10, nextPoint, precPoint);
    const ShortPoint *tangentPoint = &points[pointIndex];
    ShortPoint tangentStart, tangentEnd;
    sub(&tangentStart, tangentPoint, &fromPrecToNext);
    add(&tangentEnd,   tangentPoint, &fromPrecToNext);
    drawLine(tangentStart.x, tangentStart.y, tangentEnd.x, tangentEnd.y, (void *) tangentColor);
}


static void
drawRandomPaths(void)
{
    showDemoHelp();
    if (continueToGraphics())
        return;

    for (;;)
    {
        // Place control points at random.
        //
        ShortPoint points[NUM_RANDOM_PATH_POINTS];
        byte numPoints = NUM_RANDOM_PATH_POINTS;
        for (byte i = 0; i < numPoints; ++i)
        {
            if (!reuseSamePoints)
            {
                points[i].x = rand() % (SCREEN_WIDTH  - 20) + 10;
                points[i].y = rand() % (SCREEN_HEIGHT - 20) + 10;
            }
        }

        if (pointsSorted)
            sortPoints(points, numPoints);

        if (tangentsDrawn)
            for (byte i = 0; i < numPoints; ++i)  // draw a tangent segment over each control point
                drawTangent(points, numPoints, i);

        drawControlPoints(points, numPoints);

        if (soundEnabled && openPath)
            sound(scale[scalePlayedBackwards ? 7 : 0], 1);

        // Draw a closed path through the control points.
        // Have this call invoke onPieceEnd() after each of the curve pieces,
        // to play a musical note.
        // Have this call invoke drawLine() to display the segments that form
        // each piece. NULL is a value that drawLine() could use as local context,
        // but we have no use for thats in this case.
        //
        drawCatmullRomPath(openPath, points, numPoints, 10, onPieceEnd, drawLine, (void *) lineColor);

        if (quitRequested)
        {
            showDemoHelp();
            if (continueToGraphics())
                break;
        }

        // Wait for the user to press a key. Quit it is the Break key.
        //
        BOOL breakPressed = FALSE;
        if (autoMode)
            breakPressed = !sleepAndCheckKeys(120);
        else if (!interpretCommand(waitkey(FALSE)))
            breakPressed = TRUE;

        if (breakPressed)
        {
            showDemoHelp();
            if (continueToGraphics())
                break;
        }
        
        // Play the scale in the other order during the next iteration.
        //
        scalePlayedBackwards = !scalePlayedBackwards;

        clearGraphicsScreen();  // clear screen for next iteration
    }
}


// Polls the position of the specified joystick axis.
// axis: 0 = horizontal, 1 = vertical.
// Returns -2 (very low), -1 (medium low), 0 (center),
// +1 (medium high), +2 (very high).
// For the horizontal, low means left.
// For the vertical, low means up.
//
static char
getJoystickAxisMove(byte axisPosition)
{
    if (axisPosition < 20)
        return -1;
    if (axisPosition > 44)
        return +1;
    return 0;
}


static void
addClamped(short *dest, char diff, short minValue, short maxValue)
{
    short sum = *dest + diff;
    if (sum < minValue)
        *dest = minValue;
    else if (sum > maxValue)
        *dest = maxValue;
    else
        *dest = sum;
}


const ShortPoint cursorPixelOffsets[NUM_CURSOR_PIXELS] =
{
    {  0,  0 },  // center of cusor
    { -1,  0 },  // left of center
    { +1,  0 },
    {  0, -1 },  // above center
    {  0, +1 },
};


static void
putCursor(const ShortPoint *cursorPos, byte previousScreenContent[NUM_CURSOR_PIXELS])
{
    tuneInGraphicsBuffer();
    for (byte i = 0; i < NUM_CURSOR_PIXELS; ++i)
    {
        word x = (word) (cursorPos->x + cursorPixelOffsets[i].x);
        byte y = (byte) (cursorPos->y + cursorPixelOffsets[i].y);
        previousScreenContent[i] = (*getPixelFuncPtr)(x, y, screenBuffer);
        (*setPixelFuncPtr)(x, y, ctrlPtColor, screenBuffer);
    }
    tuneOutGraphicsBuffer();
}


static void
removeCursor(const ShortPoint *cursorPos, const byte previousScreenContent[NUM_CURSOR_PIXELS])
{
    tuneInGraphicsBuffer();
    for (byte i = 0; i < NUM_CURSOR_PIXELS; ++i)
    {
        word x = (word) (cursorPos->x + cursorPixelOffsets[i].x);
        byte y = (byte) (cursorPos->y + cursorPixelOffsets[i].y);
        (*setPixelFuncPtr)(x, y, previousScreenContent[i], screenBuffer);
    }
    tuneOutGraphicsBuffer();
}


static byte *getTextCursorAddress(void)
{
    return * (byte **) 0x0088;
}


static void setTextCursorAddress(byte *newPos)
{
    * (byte **) 0x0088 = newPos;
}


void
showJoystickModeHelp(void)
{
    clearTextScreen();
    showTextMode();
    locate(3, 1);
    printf(" USE THE JOYSTICK AND ITS ");
    locate(3, 2);
    printf(" BUTTON TO PLACE UP TO %u ", MAX_NUM_USER_PATH_POINTS);
    locate(3, 3);
    printf(" POINTS. WITH AT LEAST 4  ");
    locate(3, 4);
    printf(" POINTS, PRESS space TO   ");
    locate(3, 5);
    printf(" DRAW THE PATH.           ");
    locate(3, 7);
    printf(" WHILE DRAWING:           ");
    locate(3, 8);
    byte *leftArrow = getTextCursorAddress() + 1;
    printf(" .      REMOVE LAST POINT ");
    *leftArrow = 31;  // replace placeholder dot w/ 6847 code for reverse video left arrow
    locate(3, 9);
    printf(" clear  REMOVE ALL POINTS ");
    locate(3, 10);
    printf(" break  QUIT              ");
    locate(3, 12);
    printf(" RIGHT NOW:               ");
}


static void
drawUserPaths(void)
{
    showJoystickModeHelp();
    if (continueToGraphics())
        return;

    ShortPoint cursorPos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 }, prevCursorPos;
    byte underCursor[NUM_CURSOR_PIXELS];
    BOOL cursorDrawn = FALSE;
    BOOL buttonEnabled = TRUE;
    BOOL pathNeedsDrawn = FALSE;
    BOOL pathNeedsCleared = FALSE;
    BOOL lastPtNeedsCleared = FALSE;

    ShortPoint points[MAX_NUM_USER_PATH_POINTS];
    byte numPoints = 0;

    for (;;)
    {
        const byte *joyPositions = readJoystickPositions();  // 4-byte array of 0..63
        BOOL buttonDown = ((readJoystickButtons() & 1) == 0);
        if (!buttonEnabled && !buttonDown)
            buttonEnabled = TRUE;  // renable button now that is has been released
        char dx = getJoystickAxisMove(joyPositions[0]);
        char dy = getJoystickAxisMove(joyPositions[1]);
        byte key = inkey();
        if (key == 3)
        {
            showJoystickModeHelp();
            if (continueToGraphics())
                break;
        }

        BOOL cursorNeedsRemoved = FALSE;
        if (dx != 0 || dy != 0)
        {
            cursorNeedsRemoved = cursorDrawn;
            addClamped(&cursorPos.x, dx, MARGIN, SCREEN_WIDTH  - 1 - MARGIN);
            addClamped(&cursorPos.y, dy, MARGIN, SCREEN_HEIGHT - 1 - MARGIN);
        }
        if (buttonEnabled && buttonDown)
        {
            cursorNeedsRemoved = cursorDrawn;
        }
        if (numPoints >= 4 && key == DRAW_KEY)  // if enough points to draw a spline, and user asks to draw path
        {
            sound(1, 1);
            cursorNeedsRemoved = cursorDrawn;
            pathNeedsDrawn = TRUE;
        }
        if (key == CLEAR_KEY && numPoints > 0)
        {
            sound(1, 1);
            pathNeedsCleared = TRUE;
        }
        if (key == DEL_POINT_KEY && numPoints > 0)
        {
            sound(1, 1);
            lastPtNeedsCleared = TRUE;
        }

        if (cursorNeedsRemoved)
        {
            removeCursor(&prevCursorPos, underCursor);
            cursorDrawn = FALSE;
        }

        if (buttonEnabled && buttonDown)
        {
            // Add a control point if room left.
            if (numPoints < MAX_NUM_USER_PATH_POINTS)
            {
                points[numPoints] = cursorPos;
                //printf("SET(%u,%u). ", (word) cursorPos.x, (byte) cursorPos.y);
                //(*setPixelFuncPtr)((word) cursorPos.x, (byte) cursorPos.y, ctrlPtColor, screenBuffer);
                drawControlPoint(&points[numPoints], TRUE);
                ++numPoints;
            }
            buttonEnabled = FALSE;  // reenable only when button is released
        }

        if (pathNeedsDrawn)  // if enough points to draw a spline, and user asks to draw path
        {
            // Draw with last 4 points.
            clearGraphicsScreen();  // needed b/c curve changes path when new point added
            drawControlPoints(points, numPoints);
            drawCatmullRomPath(TRUE, points, numPoints, 10, NULL, drawLine, (void *) lineColor);
            pathNeedsDrawn = FALSE;
        }

        if (pathNeedsCleared)
        {
            numPoints = 0;
            clearGraphicsScreen();
            pathNeedsCleared = FALSE;
        }

        if (lastPtNeedsCleared)
        {
            --numPoints;
            drawControlPoint(&points[numPoints], FALSE);
            // No redraw b/c lengthy; user can press DRAW_KEY to force redraw.
            lastPtNeedsCleared = FALSE;
        }

        if (!cursorDrawn)
        {
            putCursor(&cursorPos, underCursor);
            prevCursorPos = cursorPos;
            cursorDrawn = TRUE;
        }
    }
}


typedef struct PosCouple
{
    byte *positions[MAX_BANGS];  // [0] is radio-button, [1] is key to press
} PosCouple;


// line: May contain up to MAX_BANGS exclamation points.
//       The screen address of the character that follows each exclamation point
//       is stored in the positions[] array of the returned PosCouple object.
//       Any remain entry in that array is initialized with NULL.
//
PosCouple
printMenuLine(const char *line)
{
    byte *origScreenPos = getTextCursorAddress();
    PosCouple result;
    byte numBangs = 0;
    for (char ch; (ch = *line) != '\0'; ++line)
        if (ch == '!' && numBangs < MAX_BANGS)
            result.positions[numBangs++] = getTextCursorAddress();
        else
            putchar(ch);

    for ( ; numBangs < MAX_BANGS; ++numBangs)
        result.positions[numBangs] = NULL;

    setTextCursorAddress(origScreenPos + 32);

    return result;
}


void
invertCharAt(byte *screenPos)
{
    *screenPos ^= 0x40;
}


void
setCheckMark(byte *screenPos, BOOL newState)
{
    *screenPos = (newState ? MC6847_CAPITAL_X : MC6847_SPACE);
}


// Goes to text mode and stays in it.
// coco3Graphics, randomMode: Must be initialized. Changed by the user.
//
BOOL
menu(BOOL *coco3Graphics, BOOL *randomMode)
{
    showTextMode();
    clearTextScreen();
    locate(2, isCoCo3 ? 2 : 4);  // top left position of the text
    printMenuLine("CATMULL-ROM CURVE DEMO");
    printMenuLine("BY PIERRE SARRAZIN");
    printMenuLine("");
    PosCouple cc3, pm4, rnd, joy;  // positions on the screen
    if (isCoCo3)
    {
          printMenuLine("GRAPHICS MODE:");
    cc3 = printMenuLine("  (! ) COCO !3 (256X200)");
    pm4 = printMenuLine("  (! ) PMODE !4 (256X192)");
          printMenuLine("");

        // Show the keys to press in inverted colors.
        invertCharAt(cc3.positions[1]);
        invertCharAt(pm4.positions[1]);

        setCheckMark((*coco3Graphics ? &cc3 : &pm4)->positions[0], TRUE);
    }

          printMenuLine("INTERACTION:");
    rnd = printMenuLine("  (! ) !RANDOM CURVES");
    joy = printMenuLine("  (! ) DRAW WITH !JOYSTICK");
          printMenuLine("");
    if (isCoCo3)
          printMenuLine("CHOOSE OPTIONS, PRESS ENTER.");
    else
          printMenuLine("CHOOSE OPTION, PRESS ENTER.");

    invertCharAt(rnd.positions[1]);
    invertCharAt(joy.positions[1]);

    setCheckMark((*randomMode ? &rnd : &joy)->positions[0], TRUE);

    for (;;)
    {
        byte key = waitkey(FALSE);  // no cursor
        if (key == 3)  // break
            return FALSE;
        if (key == 13)  // enter
        {
            if (isCoCo3)
                *coco3Graphics = (*cc3.positions[0] != MC6847_SPACE);
            *randomMode = (*rnd.positions[0] != MC6847_SPACE);
            return TRUE;
        }
        switch (key)
        {
        case '3':
            if (isCoCo3)
            {
                setCheckMark(cc3.positions[0], TRUE);
                setCheckMark(pm4.positions[0], FALSE);
            }
            break;
        case '4':
            if (isCoCo3)
            {
                setCheckMark(pm4.positions[0], TRUE);
                setCheckMark(cc3.positions[0], FALSE);
            }
            break;
        case 'R':
            setCheckMark(rnd.positions[0], TRUE);
            setCheckMark(joy.positions[0], FALSE);
            break;
        case 'J':
            setCheckMark(joy.positions[0], TRUE);
            setCheckMark(rnd.positions[0], FALSE);
            break;
        }
    }
}


int
main()
{
    initCoCoSupport();

    size_t pmodeBufferSize = * (byte **) 0x0019 - getPModeGraphicsBuffer();  // from PMODE buffer to start of Basic program
    if (pmodeBufferSize <= 4 * 1536)
    {
        printf("THIS PROGRAM REQUIRES 4 PAGES\n"
               "OF PMODE GRAPHICS. TYPE THIS:\n"
               "PCLEAR 4:EXEC\n");
        return 0;
    }

    coco3Graphics = isCoCo3;
    rgb();

    BOOL randomMode = TRUE;

    for (;;)
    {
        if (!menu(&coco3Graphics, &randomMode))
            break;

        srand(getTimer());  // seed after waiting for user input

        initGraphics();  // must be called here now that coco3Graphics may have been changed by menu()
        clearGraphicsScreen();

        if (randomMode)
            drawRandomPaths();
        else
            drawUserPaths();
    }

    cls(255);
    showTextMode();
    rgb();
    printf("TYPE EXEC TO RUN AGAIN.\n");
    setHighSpeed(FALSE);
    return 0;
}
