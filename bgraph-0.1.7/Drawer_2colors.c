/*  Drawer_2colors.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

const unsigned char Drawer_twoColorModeResetMasks[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

const unsigned char Drawer_twoColorModeColorMasks[2] =
{
    0x00,   // color index 0: all 8 pixels are black
    0xFF,   // color index 1: all 8 pixels are white/green
};
