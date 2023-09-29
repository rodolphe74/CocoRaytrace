// testclip.c - Program that tests clipping.c.

#include "clipping_private.h"

#include <assert.h>

#define assert_eq(actual, expected) \
do { \
    /*printf("assert_eq(%s, %s): %d, %d\n", #actual, #expected, actual, expected);*/ \
    if ((actual) != (expected)) { \
         printf("ERROR: assert_eq failed: line %d: should be equal: got %u ($%x), expected %u ($%x)\n", \
                __LINE__, (word) (actual), (word) (actual), (word) (expected), (word) (expected)); \
         exit(1); \
   } } while (0)


int
main()
{
    ShortClipRect rect = { { 4, 4 }, { 10, 8 } };

    assert_eq(compOutCode(7, 6, &rect), INSIDE);
    assert_eq(compOutCode(7, 3, &rect), TOP);
    assert_eq(compOutCode(7, 9, &rect), BOTTOM);

    short x0, y0, x1, y1;
    x0 = 5, y0 = 5, x1 = 7, y1 = 7;
    BOOL a0 = cohenSutherlandLineClip(&x0, &y0, &x1, &y1, &rect);
    x0 = 7, y0 = 9, x1 = 11, y1 = 4;
    BOOL a1 = cohenSutherlandLineClip(&x0, &y0, &x1, &y1, &rect);
    x0 = 1, y0 = 5, x1 = 4, y1 = 1;
    BOOL a2 = cohenSutherlandLineClip(&x0, &y0, &x1, &y1, &rect);
    //printf("%u %u %u\n", a0, a1, a2);
    assert(a0);
    assert(a1);
    assert(!a2);

    //  0           B
    //  1          /
    //  2         /
    //  3  +--+--D--+
    //  4  |  | /|  |
    //  5  |  |/ |  |
    //  6  +--C--+--+
    //  7    /
    //  8   /
    //  9  A
    //     0123456789
    //
    // Clipping diagonal line AB along the above  rectangle gives line CD.
    //
    x0 = 0, y0 = 9;  // point A
    x1 = 9, y1 = 0;  // point B
    const ShortClipRect rect0 = { { 0, 3 }, { 9, 6 } };
    BOOL a3 = cohenSutherlandLineClip(&x0, &y0, &x1, &y1, &rect0);
    assert(a3);
    assert_eq(x0, 3);
    assert_eq(y0, 6);
    assert_eq(x1, 6);
    assert_eq(y1, 3);

    // More tests.
    //
    const ShortPoint expected[] =
    {
        {  5, 4 },
        {  7, 4 },
        {  9, 4 },
        {  4, 6 },
        {  7, 6 },
        { 10, 6 },
        {  5, 8 },
        {  7, 8 },
        {  9, 8 },
    };
    byte index = 0;
    for (short y = 3; y <= 9; y += 3)
        for (short x = 3; x <= 11; x += 4, ++index)
        {
            //printf("index: %u\n", index);
            short x0 = 7, y0 = 6, x1 = x, y1 = y;
            //printf("%2d %2d -> ", x1, y1);

            assert_eq(compOutCode(x0, y0, &rect), INSIDE);
            assert_eq(compOutCode(x1, y1, &rect), (x == 3 ? LEFT : (x == 11 ? RIGHT : INSIDE)) | (y == 3 ? TOP : (y == 9 ? BOTTOM : INSIDE)));

            BOOL accepted = cohenSutherlandLineClip(&x0, &y0, &x1, &y1, &rect);
            //printf("%2d %2d %2d %2d %u\n", x0, y0, x1, y1, accepted);
            assert(accepted);
            assert_eq(x0, 7);
            assert_eq(y0, 6);
            assert_eq(x1, expected[index].x);
            assert_eq(y1, expected[index].y);
        }

    printf("UNIT TESTS PASSED.\n");
    return 0;
}
