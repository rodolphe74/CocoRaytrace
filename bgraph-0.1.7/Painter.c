/*  Painter.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "Painter.h"


enum
{
    ITERATIONS_PER_CONT_FUNC_INVOCATION = 10,
};


static BOOL
Painter_push(Painter *p, word a, byte b)
{
    if (p->stackCapacity - p->stackSize < 3)  // if fewer than 3 bytes left
        return FALSE;
    byte *ptr = p->stackArray + p->stackSize;
    * (word *) ptr = a;
    ptr[2] = b;
    p->stackSize += 3;
    return TRUE;
}


static BOOL
Painter_pop(Painter *p, word *a, byte *b)
{
    if (p->stackSize < 3)
        return FALSE;
    p->stackSize -= 3;
    byte *ptr = p->stackArray + p->stackSize;
    *a = * (word *) ptr;
    *b = ptr[2];
    return TRUE;
}


static void
Painter_popAll(Painter *p)
{
    p->stackSize = 0;
}


BOOL
Painter_init(Painter *p,
             Painter_getPixelFuncPtr getPixel,
             Painter_setPixelFuncPtr setPixel,
             Painter_continuationFuncPtr continuationFuncPtr,
             word screenWidthInPixels,
             byte screenHeightInPixels,
             byte *workSpace,
             size_t workSpaceCapacity)
{
    if (p == NULL || getPixel == NULL || setPixel == NULL
            || screenWidthInPixels == 0 || screenHeightInPixels == 0
            || workSpace == NULL || workSpaceCapacity == 0)
        return FALSE;
    p->getPixel = getPixel;
    p->setPixel = setPixel;
    p->screenWidthInPixels = screenWidthInPixels;
    p->screenHeightInPixels = screenHeightInPixels;
    p->continuationFuncPtr = continuationFuncPtr;
    p->stackArray = workSpace;
    p->stackCapacity = workSpaceCapacity;
    p->stackSize = 0;
    return TRUE;
}


static BOOL
Painter_alwaysContinue(void *cookie)
{
    return TRUE;
}


byte
Painter_paint(Painter *p,
              word x, byte y,
              byte newColor, byte oldColor,
              void *cookie)
{
    if (oldColor == newColor)
        return Painter_INVALID_ARGUMENT;

    BOOL spanAbove, spanBelow;

    p->stackSize = 0;
    if (!Painter_push(p, x, y))
        return Painter_OUT_OF_STACK;

    // If no continuation callback provided, use one that always lets the painting continue.
    Painter_continuationFuncPtr contFuncPtr = (p->continuationFuncPtr != NULL
                                                ? p->continuationFuncPtr
                                                : Painter_alwaysContinue);

    // Cache some values.
    Painter_getPixelFuncPtr getPixel = p->getPixel;
    Painter_setPixelFuncPtr setPixel = p->setPixel;
    word w = p->screenWidthInPixels;
    byte hMinus1 = p->screenHeightInPixels - 1;

    byte contFuncInvocationCounter = ITERATIONS_PER_CONT_FUNC_INVOCATION;
    while (Painter_pop(p, &x, &y))
    {
        while ((int) x >= 0 && getPixel(x, y, cookie) == oldColor)
            --x;
        ++x;
        spanAbove = FALSE;
        spanBelow = FALSE;
        while (x < w && getPixel(x, y, cookie) == oldColor)
        {
            setPixel(x, y, newColor, cookie);
            if (!spanAbove && y > 0 && getPixel(x, y - 1, cookie) == oldColor)
            {
                if (!Painter_push(p, x, y - 1))
                    return Painter_OUT_OF_STACK;
                spanAbove = TRUE;
            }
            else if (spanAbove && y > 0 && getPixel(x, y - 1, cookie) != oldColor)
            {
                spanAbove = FALSE;
            }
            if (!spanBelow && y < hMinus1 && getPixel(x, y + 1, cookie) == oldColor)
            {
                if (!Painter_push(p, x, y + 1))
                    return Painter_OUT_OF_STACK;
                spanBelow = TRUE;
            }
            else if (spanBelow && y < hMinus1 && getPixel(x, y + 1, cookie) != oldColor)
            {
                spanBelow = FALSE;
            }
            ++x;
        }

        if (--contFuncInvocationCounter == 0)
        {
            if (! (*contFuncPtr)(cookie))  // if callbacks returns FALSE, stop
            {
                Painter_popAll(p);
                break;
            }

            contFuncInvocationCounter = ITERATIONS_PER_CONT_FUNC_INVOCATION;
        }
    }
    
    return Painter_NO_ERROR;
}
