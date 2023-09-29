/*  Drawer.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "Drawer.h"


#define tracef(...) ((void) 0)
//#define tracef printf

#ifdef NDEBUG
#define assert(cond) ((void) 0)
#else
#define assert(cond) do { if (!(cond)) { \
                            screen(0, 0); \
                            (printf)("\nASSERT FAILED AT LINE %d\n", __LINE__); \
                            exit(1); \
                     } } while (FALSE)
#endif


// Reset masks for each pixel in a screen byte in 2-color mode.
// Suitable for Drawer_init() and a mode like PMODE 4.
//
extern const byte Drawer_twoColorModeResetMasks[8];


// Reset masks for each pixel in a screen byte in 4-color mode.
// Suitable for Drawer_init() and a mode like PMODE 3.
//
extern const byte Drawer_fourColorModeResetMasks[4];


// Reset masks for each pixel in a screen byte in 2-color mode.
// Suitable for Drawer_init() and a mode like HSCREEN 2 on the CoCo 3.
//
extern const byte Drawer_sixteenColorModeResetMasks[2];


// Color masks for each color of a 2-color mode.
// Suitable for Drawer_init() and a mode like PMODE 4.
//
extern const byte Drawer_twoColorModeColorMasks[2];


// Color masks for each color of a 4-color mode.
// Suitable for Drawer_init() and a mode like PMODE 3.
//
extern const byte Drawer_fourColorModeColorMasks[4];


// Color masks for each color of a 16-color mode.
// Suitable for Drawer_init() and a mode like HSCREEN 2 on the CoCo 3.
//
extern const byte Drawer_sixteenColorModeColorMasks[16];


// Sets the current drawing color by setting the current color mask.
// newColorIndex: Zero-based index into the color mask array passed to Drawer_init().
//
void
Drawer_setColorIndex(Drawer *d, byte newColorIndex)
{
    d->colorIndex = newColorIndex;
    d->colorMask = d->colorMaskArray[newColorIndex];
    tracef("SCI %02x > %02x %02x\n", newColorIndex, d->colorIndex, d->colorMask);
}


// x, y: Pixel positions.
//
static void
Drawer_initPtr(Drawer *d, word x, byte y)
{
    d->ptr = d->screenBegin + (word) d->bytesPerRow * y + x / d->pixelsPerByte;
    assert(d->ptr >= d->screenBegin && d->ptr < d->screenEnd);
}


BOOL
Drawer_init(Drawer *d,
            byte *screenBuffer,
            word screenWidthInPixels, byte screenHeightInPixels,
            byte pixelsPerByte)
{
    tracef("INIT(%p,%u,%u,%u)\n", screenBuffer, screenWidthInPixels, screenHeightInPixels, pixelsPerByte);
    d->screenBegin = screenBuffer;
    d->screenEnd = screenBuffer + screenWidthInPixels * screenHeightInPixels / pixelsPerByte;
    d->screenWidthInPixels = screenWidthInPixels;
    d->screenHeightInPixels = screenHeightInPixels;
    d->bytesPerRow = (byte) (screenWidthInPixels / pixelsPerByte);
    d->pixelsPerByte = pixelsPerByte;
    d->xPos = screenWidthInPixels / 2;
    d->yPos = screenHeightInPixels / 2;
    Drawer_initPtr(d, d->xPos, d->yPos);

    byte initColorIndex;
    switch (pixelsPerByte)
    {
    case 8:
        d->resetMaskArray = Drawer_twoColorModeResetMasks;
        d->colorMaskArray = Drawer_twoColorModeColorMasks;
        initColorIndex = 1;
        break;
    case 4:
        d->resetMaskArray = Drawer_fourColorModeResetMasks;
        d->colorMaskArray = Drawer_fourColorModeColorMasks;
        initColorIndex = 3;
        break;
    case 2:
        d->resetMaskArray = Drawer_sixteenColorModeResetMasks;
        d->colorMaskArray = Drawer_sixteenColorModeColorMasks;
        initColorIndex = 15;
        break;
    default:
        return FALSE;
    }
    
    // Now that colorMaskArray field is initialized:
    Drawer_setColorIndex(d, initColorIndex);

    return TRUE;
}


// Returns the reset mask that applies to the current horizontal pixel position.
//
static byte
Drawer_getResetMask(Drawer *d)
{
    return d->resetMaskArray[d->xPos & (d->pixelsPerByte - 1)];
}


// Reads and returns an unsigned decimal number at *pp, and leaves *pp pointing
// after the number.
// Starts by skipping spaces if any.
// If no decimal digit is seen at *pp, the latter is not advanced
// and 'defaultValue' is returned.
//
static word
Drawer_readUIntDef(const char **pp, word defaultValue)
{
    const char *p = *pp;
    while (*p == ' ')
        ++p;
    word value = 0;
    for (;;)
    {
        char c = *p++;
        if (c < '0' || c > '9')
            break;
        value = value * 10 + (c - '0');
    }
    if (p == *pp + 1)
        value = defaultValue;
    else
        *pp = p - 1;
    return value;
}


#define Drawer_readUInt(pp) Drawer_readUIntDef(pp, 1)


// Sets the color of the current pixel to the color specified by d->colorMask,
// using the given reset mask, where a set bit represents a bit position to
// be affected.
// Example: Drawer_setPixelWithResetMask(d, 0x0C) with d->colorMask == 0xFF
//          will set bits 2 and 3 of the current byte to 1.
//
static void
Drawer_setPixelWithResetMask(Drawer *d, byte resetMask)
{
    assert(d->ptr >= d->screenBegin && d->ptr < d->screenEnd);
    byte newByte = (*d->ptr & ~resetMask) | (d->colorMask & resetMask);
    tracef("SP %p %02x %02x %02x>%02x\n", d->ptr, resetMask, d->colorMask, *d->ptr, newByte);
    *d->ptr = newByte;
}


// Sets the color of the current pixel to the color specified by d->colorMask.
//
static void
Drawer_setPixelXPos(Drawer *d)
{
    Drawer_setPixelWithResetMask(d, Drawer_getResetMask(d));
}


// Increments the current vertical position by one.
// Returns TRUE if the current vertical position is still valid.
//
static BOOL
Drawer_moveDown(Drawer *d)
{
    ++d->yPos;
    d->ptr += d->bytesPerRow;
    return d->ptr < d->screenEnd;
}


// Decrements the current vertical position by one.
// Returns TRUE if the current vertical position is still valid.
//
static BOOL
Drawer_moveUp(Drawer *d)
{
    --d->yPos;
    d->ptr -= d->bytesPerRow;
    return d->ptr >= d->screenBegin;
}


// Increments the current horizontal position by one.
// Returns TRUE if the current horizontal position is still valid.
//
static BOOL
Drawer_moveRight(Drawer *d)
{
    ++d->xPos;
    BOOL newPosIsValid = (d->xPos < d->screenWidthInPixels);
    if ((d->xPos & (d->pixelsPerByte - 1)) == 0)  // if now at left of another byte
        ++d->ptr;
    return newPosIsValid;
}


// Derements the current horizontal position by one.
// Returns TRUE if the current horizontal position is still valid.
//
static BOOL
Drawer_moveLeft(Drawer *d)
{
    --d->xPos;
    BOOL newPosIsValid = ((sword) d->xPos >= 0);
    byte m = d->pixelsPerByte - 1;
    if ((d->xPos & m) == m)  // if now at right of another byte
        --d->ptr;
    return newPosIsValid;
}


// Used by Primitive_line().
// cookie: Must point to a Drawer object.
//
static void
Drawer_plot(word x, byte y, byte colorIndex, void *cookie)
{
    tracef("PLOT(%u,%u,%u,%p)\n", x, y, colorIndex, cookie);
    Drawer *d = (Drawer *) cookie;
    assert(d->ptr >= d->screenBegin && d->ptr < d->screenEnd);
    byte *ptr = d->screenBegin + (word) y * d->bytesPerRow + x / d->pixelsPerByte;
    byte resetMask = d->resetMaskArray[(byte) x & (d->pixelsPerByte - 1)];
    byte colorMask = d->colorMaskArray[colorIndex];
    *ptr = (*ptr & ~resetMask) | (colorMask & resetMask);
}


// Draws in the direction specified by forwardHorizMoveFunc and forwardVertMoveFunc.
// The other two function pointers are used to back up when needed.
// scriptPtr: Must point to an optional integer giving the number of pixels to move forward by.
//            If no integer is at the address, 1 is used.
//            *scriptPtr gets updated to point after the integer, if one is read.
// blank: If TRUE, no pixels are drawn, only the position is advanced.
//
static void
Drawer_drawAt45Degrees(Drawer *d,
                       const char **scriptPtr,
                       BOOL blank,
                       BOOL (*forwardHorizMoveFunc)(Drawer *),
                       BOOL (*backwardHorizMoveFunc)(Drawer *),
                       BOOL (*forwardVertMoveFunc)(Drawer *),
                       BOOL (*backwardVertMoveFunc)(Drawer *))
{
    word len = Drawer_readUInt(scriptPtr) + 1;
    BOOL horizMoveFailed = FALSE;
    while (len--)
    {
        if (!blank)
            Drawer_setPixelXPos(d);
        if (!forwardHorizMoveFunc(d))
        {
            horizMoveFailed = TRUE;
            break;
        }
        if (!forwardVertMoveFunc(d))
            break;
    }
    backwardHorizMoveFunc(d);
    if (!horizMoveFailed)
        backwardVertMoveFunc(d);
}


static int
clampInt(int n, int min, int max)
{
    if (n < min)
        return min;
    if (n > max)
        return max;
    return n;
}


static const word Drawer_maxColorsInMode[9] = { 0, 256, 16, 0, 4, 0, 0, 0, 2 };


const char *
Drawer_draw(Drawer *d, const char *script)
{
    BOOL blank = FALSE, updatePos = TRUE;

    tracef("INIT SCREEN PTR: %p\n", d->screenBegin);

    for (;;)
    {
        tracef("CMD S%p '%02x P%p X%02x\n", script, *script, d->screenBegin, d->xPos);

        // Support for B (blank).
        //
        BOOL nextCmdIsBlank = FALSE;

        // Support for N (no update).
        //
        BOOL nextCmdUpdatesPos = TRUE;
        byte *origPtr;
        word origXPos;
        byte origYPos;
        if (!updatePos)
        {
            origPtr = d->ptr;
            origXPos = d->xPos;
            origYPos = d->yPos;
        }

        // Interpret current command.
        //
        while (*script == ' ' || *script == ';')
            ++script;

        switch (*script++)
        {
        case '\0':
            return 0;  // success
        case 'U':
        {
            byte resetMask = Drawer_getResetMask(d);
            word len = Drawer_readUInt(&script) + 1;
            while (len--)
            {
                if (!blank)
                    Drawer_setPixelWithResetMask(d, resetMask);
                if (!Drawer_moveUp(d))
                    break;
            }
            Drawer_moveDown(d);
            break;
        }
        case 'D':
        {
            byte resetMask = Drawer_getResetMask(d);
            word len = Drawer_readUInt(&script) + 1;
            while (len--)
            {
                if (!blank)
                    Drawer_setPixelWithResetMask(d, resetMask);
                if (!Drawer_moveDown(d))
                    break;
            }
            Drawer_moveUp(d);
            break;
        }
        case 'E':
            Drawer_drawAt45Degrees(d, &script, blank,
                                  Drawer_moveRight, Drawer_moveLeft,
                                  Drawer_moveUp, Drawer_moveDown);
            break;
        case 'F':
            Drawer_drawAt45Degrees(d, &script, blank,
                                  Drawer_moveRight, Drawer_moveLeft,
                                  Drawer_moveDown, Drawer_moveUp);
            break;
        case 'G':
            Drawer_drawAt45Degrees(d, &script, blank,
                                  Drawer_moveLeft, Drawer_moveRight,
                                  Drawer_moveDown, Drawer_moveUp);
            break;
        case 'H':
            Drawer_drawAt45Degrees(d, &script, blank,
                                  Drawer_moveLeft, Drawer_moveRight,
                                  Drawer_moveUp, Drawer_moveDown);
            break;
        case 'R':
        {
            word len = Drawer_readUInt(&script) + 1;
            while (len--)
            {
                if (!blank)  // Case where 'blank' is TRUE, here and in 'L', could be optimized.
                    Drawer_setPixelXPos(d);
                if (!Drawer_moveRight(d))
                    break;
            }
            Drawer_moveLeft(d);
            break;
        }
        case 'L':
        {
            word len = Drawer_readUInt(&script) + 1;
            while (len--)
            {
                if (!blank)
                    Drawer_setPixelXPos(d);
                if (!Drawer_moveLeft(d))
                    break;
            }
            Drawer_moveRight(d);
            break;
        }
        case 'B':
            nextCmdIsBlank = TRUE;
            break;
        case 'N':
            nextCmdUpdatesPos = FALSE;
            break;
        case 'M':
        {
            BOOL relative = FALSE, negative = FALSE;
            if (*script == '+' || *script == '-')
            {
                relative = TRUE;
                negative = (*script == '-');
                ++script;
            }
            word x = Drawer_readUIntDef(&script, 0);
            if (negative)
                x = -x;
            if (*script == ',')
            {
                ++script;
                if (*script == '+' || *script == '-')
                {
                    if (!relative)
                        return script;
                    negative = (*script == '-');
                    ++script;
                }
                word y = Drawer_readUIntDef(&script, 0);
                if (negative)
                    y = -y;
                if (relative)
                {
                    x = (word) ((sword) d->xPos + (sword) x);
                    y = (word) ((sword) d->yPos + (sword) y);
                }
                x = (word) clampInt((int) x, 0, (int) (d->screenWidthInPixels - 1));
                y = (word) clampInt((int) y, 0, (int) (d->screenHeightInPixels - 1));
                if (!blank)
                    Primitive_line(d->xPos, d->yPos, x, (byte) y, d->colorIndex, Drawer_plot, d);
                d->xPos = x;
                d->yPos = (byte) y;
                Drawer_initPtr(d, x, (byte) y);
            }
            break;
        }
        case 'C':
        {
            const char *origScript = script;
            word newColorIndex = Drawer_readUIntDef(&script, 0xFFFF);
            if (newColorIndex >= Drawer_maxColorsInMode[d->pixelsPerByte])
                return origScript;
            Drawer_setColorIndex(d, (byte) newColorIndex);
            break;
        }
        default:
            return script - 1;  // point to unknown command character
        }

        // Support for 'N'.
        //
        if (!updatePos)
        {
            d->ptr = origPtr;
            d->xPos = origXPos;
            d->yPos = origYPos;
        }

        blank = nextCmdIsBlank;
        updatePos = nextCmdUpdatesPos;
    }
    return 0;
}


void
Drawer_getPosition(Drawer *d, word *pX, byte *pY)
{
    *pX = d->xPos;
    *pY = d->yPos;
}


void
Drawer_setPosition(Drawer *d, word x, byte y)
{
    d->xPos = x;
    d->yPos = y;
    Drawer_initPtr(d, x, y);
}
