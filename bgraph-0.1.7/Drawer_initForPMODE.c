/*  Drawer_initForPMODE.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "Drawer.h"


BOOL
Drawer_initForPMODE(Drawer *d, byte *screenBuffer, byte pmodeNo)
{
    if (pmodeNo > 4)
        return FALSE;
    return Drawer_init(d, screenBuffer,
                       pmodeNo == 4 ? 256 : 128,
                       pmodeNo <= 1 ? 96 : 192,
                       pmodeNo & 1 ? 4 : 8);
}
