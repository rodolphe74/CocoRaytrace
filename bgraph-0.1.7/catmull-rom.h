/*  clipping.h - Public definitions to draw Catmull-Rom curves.
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#ifndef _H_catmull_rom
#define _H_catmull_rom

#include "clipping.h"  /* bgraph */


// Pointer type for a function to be invoked by drawCatmullRomPath() before drawing
// each piece of the path.
// pointIndex will be the zero-based index into the 'points' array passed to drawCatmullRomPath().
// cookie will be the cookie parameter passed to drawCatmullRomPath().
// Must return TRUE to tell drawCatmullRomPath() to continue drawing,
// or FALSE to tell it to stop.
//
typedef BOOL (*CatmullRom_onPieceEnd)(size_t pointIndex, void *cookie);

// Pointer type for a function to be invoked by drawCatmullRomPath() to draw a line segment
// from (x0, y0) to (x1, y1).
// cookie will be the cookie parameter passed to drawCatmullRomPath().
// Must return TRUE to tell drawCatmullRomPath() to continue drawing,
// or FALSE to tell it to stop.
//
typedef BOOL (*CatmullRom_drawLine)(short x0, short y0, short x1, short y1, void *cookie);


// openPath: If TRUE, the last point is NOT tied to the first point; if FALSE, those two points are tied.
// points: Array of points through which the curve must pass.
// numPoints: Number of points in points[]. At least 4.
// numSegmentsPerPiece: Must be at least 2.
// onPieceEnd: Allowed to be null. See CatmullRom_onPieceEnd.
// drawLine: Must not be null. See CatmullRom_drawLine.
// Returns TRUE if the drawing was done completely, or FALSE if a callback
// told this function to stop drawing.
//
BOOL drawCatmullRomPath(BOOL openPath, const ShortPoint *points, size_t numPoints,
                        byte numSegmentsPerPiece,
                        CatmullRom_onPieceEnd onPieceEnd,
                        CatmullRom_drawLine drawLine, void *cookie);


#endif  /* _H_catmull_rom */
