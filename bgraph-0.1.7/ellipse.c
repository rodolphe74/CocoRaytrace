/*  ellipse.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/


#include "Primitive.h"


typedef struct CosSinFactorCouple { word xFact, yFact; } CosSinFactorCouple;


// cosines and sines multipled by 65535.
//
static const CosSinFactorCouple cosSinTable[4] =
{
    { 65535,     0 },  //  0 degrees
    { 63301, 16961 },  // 15 degrees
    { 56754, 32767 },  // 30 degrees
    { 46340, 46340 },  // 45 degrees
};


typedef struct Couple { word x, y; } Couple;


typedef struct EllipsisDrawer
{
    word x;
    byte y;
    byte radius;
    byte color;
    word horizontalFactor;
    word verticalFactor;
    Primitive_setPixelFuncPtr setPixel;
    void *cookie;
    byte iteration;
    byte set;
    Couple curvePoints[2][8];
} EllipsisDrawer;


static void
EllipsisDrawer_iterate(EllipsisDrawer *drawer)
{
    const CosSinFactorCouple *c = &cosSinTable[drawer->iteration];
    word dx  = (word) ((((uint32_t) drawer->radius * drawer->horizontalFactor >> 8) * c->xFact) >> 16);
    word dy  = (word) ((((uint32_t) drawer->radius * drawer->verticalFactor   >> 8) * c->yFact) >> 16);
    word dx2 = (word) ((((uint32_t) drawer->radius * drawer->verticalFactor   >> 8) * c->xFact) >> 16);
    word dy2 = (word) ((((uint32_t) drawer->radius * drawer->horizontalFactor >> 8) * c->yFact) >> 16);

    byte set = drawer->set;
    Couple *curSet = drawer->curvePoints[set];
    const word x = drawer->x, y = drawer->y;
    curSet[0].x = x + dx;
    curSet[0].y = y + dy;
    curSet[1].x = x - dx;
    curSet[1].y = y + dy;
    curSet[2].x = x - dx;
    curSet[2].y = y - dy;
    curSet[3].x = x + dx;
    curSet[3].y = y - dy;
    curSet[4].x = x + dy2;
    curSet[4].y = y + dx2;
    curSet[5].x = x - dy2;
    curSet[5].y = y + dx2;
    curSet[6].x = x - dy2;
    curSet[6].y = y - dx2;
    curSet[7].x = x + dy2;
    curSet[7].y = y - dx2;

    if (drawer->iteration > 0)  // if both curvePoints[0] and curvePoints[1] are defined
    {
        const Couple *prevSet = drawer->curvePoints[1 - set];
        for (byte c = 0; c < 8; ++c)
            Primitive_line(prevSet[c].x, (byte) prevSet[c].y,
                           curSet[c].x,  (byte) curSet[c].y,
                           drawer->color, drawer->setPixel, drawer->cookie);
    }
    
    drawer->set = set ^ 1;  // point to other set in drawer->curvePoints[]
}


void
Primitive_ellipse(word x, byte y, byte radius, byte color,
                  word horizontalFactor, word verticalFactor,
                  Primitive_setPixelFuncPtr setPixel, void *cookie)
{
    EllipsisDrawer drawer = { x, y, radius, color,
                              horizontalFactor, verticalFactor,
                              setPixel, cookie, 0, 0 };
    for ( ; drawer.iteration < sizeof(cosSinTable) / sizeof(cosSinTable[0]); ++drawer.iteration)
        EllipsisDrawer_iterate(&drawer);
}
