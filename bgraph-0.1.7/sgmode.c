/*  sgmode.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "semigr.h"


BOOL
sgMode(byte mode, void *screenBuffer /*, SetPixelFunction *setPixelFunction, GetPixelFunction *getPixelFunction*/)
{
    byte samVRegisterValue;
    switch (mode)
    {
    case 0:
        samVRegisterValue = 0;
        break;
    case 8:
        samVRegisterValue = 2;
        break;
    case 12:
        samVRegisterValue = 4;
        break;
    case 24:
        samVRegisterValue = 6;
        break;
    default:
        return FALSE;
    }

    * (byte *) 0xFF22 &= 7;
    showGraphicsAddress(samVRegisterValue, (byte) ((word) screenBuffer >> 9));
    return TRUE;
}
