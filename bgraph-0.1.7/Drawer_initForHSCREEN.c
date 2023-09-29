/*  Drawer_initForHSCREEN.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "Drawer.h"


BOOL
Drawer_initForHSCREEN(Drawer *d, byte *screenBuffer, byte modeNo)
{
    if (modeNo < 1 || modeNo > 4)
        return FALSE;
    return Drawer_init(d, screenBuffer,
                       modeNo <= 2 ? 320 : 640,
                       192,
                       modeNo == 1 || modeNo == 4 ? 4 : (modeNo == 2 ? 2 : 8));
}
