/*  drawCatmullRomPath.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "catmull-rom.h"

#include "clipping.h"  /* bgraph */


enum
{
    NUM_CTRL_PTS = 4,
};


typedef struct TValues
{
    byte  step;
    short step2;
    long  step3;
    byte maxStep;
    short maxStep2;
    long  maxStep3;
} TValues;


// See Foley, van Dam, Feiner, Hughes, "Computer Graphics: Principles and Practice",
// 2nd Edition, pp. 483, 505.
//
static short
applyBasisMatrix(TValues *tValues, short p0, short p1, short p2, short p3)
{
    return (short) (
                      (2 * p1) +
                      (- p0 + p2) * tValues->step / tValues->maxStep +
                      (2 * p0 - 5 * p1 + 4 * p2 - p3) * (long) tValues->step2 / tValues->maxStep2 +
                      (- p0 + 3 * p1 - 3 * p2 + p3) * tValues->step3 / tValues->maxStep3
                   ) / 2;
}


// Number of segments to draw to form the piece. Must be at least 2.
// Returns FALASE if the drawLine callback returned FALSE.
// Returns TRUE otherwise.
//
static BOOL
drawCatmullRomPiece(const ShortPoint p[NUM_CTRL_PTS], byte maxStep,
                     CatmullRom_drawLine drawLine, void *cookie)
{
    short lastX, lastY;
    TValues tValues;
    for (byte step = 0; step <= maxStep; ++step)
    {
        tValues.step = step;
        tValues.step2 = (short) step * step;
        tValues.step3 = (long) tValues.step2 * step;
        tValues.maxStep = maxStep;
        tValues.maxStep2 = (short) maxStep * maxStep;
        tValues.maxStep3 = (long) tValues.maxStep2 * maxStep;

        short x = applyBasisMatrix(&tValues, p[0].x, p[1].x, p[2].x, p[3].x);
        short y = applyBasisMatrix(&tValues, p[0].y, p[1].y, p[2].y, p[3].y);
        if (step != 0)  // if not first point in curve
            if (!(*drawLine)(lastX, lastY, x, y, cookie))
                return FALSE;  // callback asked to stop drawing

        lastX = x;
        lastY = y;
    }
    return TRUE;
}


static void
incrementIndex(size_t *index, size_t increment, size_t numPoints)
{
    *index += increment;
    if (*index >= numPoints)
        *index -= numPoints;
}


BOOL
drawCatmullRomPath(BOOL openPath, const ShortPoint *points, size_t numPoints,
                   byte numSegmentsPerPiece,
                   CatmullRom_onPieceEnd onPieceEnd,
                   CatmullRom_drawLine drawLine, void *cookie)
{
    size_t limit = numPoints;
    if (openPath)
        --limit;  // do not draw piece from points[numPoints - 1] to points[0]

    for (size_t i = 0; i < limit; ++i)
    {
        ShortPoint p[NUM_CTRL_PTS];  // piece control points
        size_t j = 0;
        if (i > 0 || !openPath)
            incrementIndex(&j, i + numPoints - 1, numPoints);
        p[0] = points[j];
        if (i > 0 || !openPath)  // for open path, j stays at 0 when i is 0
            incrementIndex(&j, 1, numPoints);
        p[1] = points[j];
        incrementIndex(&j, 1, numPoints);
        p[2] = points[j];
        if (i != numPoints - 2 || !openPath)  // for open path, j stays at current value when i is last point
            incrementIndex(&j, 1, numPoints);
        p[3] = points[j];

        if (!drawCatmullRomPiece(p, numSegmentsPerPiece, drawLine, cookie))
            return FALSE;  // drawLine callback asked to stop drawing

        if (onPieceEnd && !(*onPieceEnd)(i, cookie))
            return FALSE;  // callback asked to stop drawing
    }
    return TRUE;
}
