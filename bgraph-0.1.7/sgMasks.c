/*  sgMasks.c
    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

/*
    Semigraphics-8 layout:

    Screen      4-byte
    block:      group:

    L1 L0       1 C2 C1 C0 L1 L0 -- --
    L3 L2       1 C2 C1 C0 L3 L2 -- --
    L5 L4       1 C2 C1 C0 -- -- L5 L4
    L7 L6       1 C2 C1 C0 -- -- L7 L6

    Similar layout for SG12 and SG24.
*/
const unsigned char sgMasks[2][2] =
{
    // x & 1 is 0:      x & 1 is 1:
    {  0b00001000,      0b00000100  },  // SG8: y % 4 is 0 or 1
    {  0b00000010,      0b00000001  },  // SG8: y % 4 is 2 or 3
};
