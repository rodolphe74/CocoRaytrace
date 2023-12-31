#include "errdiff.h"

color_t palette[] = {
    {2, 2, 2},
    {74, 2, 0},
    {74, 74, 172},
    {74, 74, 74},
    {172, 74, 2},
    {74, 172, 2},
    {253, 74, 2},
    {172, 172, 172},
    {2, 172, 253},
    {235, 172, 74},
    {74, 172, 172},
    {74, 253, 74},
    {253, 172, 2},
    {2, 253, 253},
    {172, 172, 2},
    {253, 253, 253}
};

FLOATING FS_STANDARD[] = {
    4.0,
    1.0, 0.0,  0.4375,
    -1.0, 1.0,  0.1875,
    0.0, 1.0,  0.3125,
    1.0, 1.0,  0.0625,
    0.0
};

/*
	public int fastDifferenceTo(VectorRGB other) {
		VectorRGB difference = this.subtract(other);
		return Math.abs(difference.r) + Math.abs(difference.g) + Math.abs(difference.b);
	}*/

#define ABS(N)((N<0)?(-N):(N))

int findNearestColor(color_t c, char multiply)
{
    int minIndex = 0;
    FLOATING minDistance = 400000.0;
    FLOATING distance = 0;

    if (multiply) {
        c.r *= 255.0;
        c.g *= 255.0;
        c.b *= 255.0;
    }

    for (unsigned char i = 0; i < 16; i++) {
        distance = (
                       (c.r - palette[i].r) * (c.r - palette[i].r) +
                       (c.g - palette[i].g) * (c.g - palette[i].g) +
                       (c.b - palette[i].b) * (c.b - palette[i].b) /*^0.5*/
                   );

        // distance = ABS(c.r - palette[i].r) +  ABS(c.g - palette[i].g) + ABS(c.b - palette[i].b);


        if (distance < minDistance) {
            minIndex = i;
            minDistance = distance;
        }
    }
    return minIndex;
}

color_t multcol(color_t c)
{
    c.r *= 255.0;
    c.g *= 255.0;
    c.b *= 255.0;
    return c;
}

color_t clip(color_t c)
{
    if (c.r > 255) c.r = 255;
    if (c.g > 255) c.g = 255;
    if (c.b > 255) c.b = 255;
    if (c.r < 0) c.r = 0;
    if (c.g < 0) c.g = 0;
    if (c.b < 0) c.b = 0;
    return c;
}

color_t applyError(color_t co, short *qe, FLOATING f)
{
    co.r = (co.r + qe[0] * f);
    co.g = (co.g + qe[1] * f);
    co.b = (co.b + qe[2] * f);
    co = clip(co);
    return co;
}

void errorDiffusiongDither(GraphicsDriver *gd, word x, byte y, color_t *co)
{

    short quantError[3];
    color_t currentColor, closestMatch, screenPixel;
    int cidx;
    for (int i = 0; i < 320; i++) {
        currentColor = co[i];

        cidx  = findNearestColor(currentColor, 0);
        closestMatch = palette[cidx];

        quantError[0] = (short)currentColor.r - (short)closestMatch.r;
        quantError[1] = (short)currentColor.g - (short)closestMatch.g;
        quantError[2] = (short)currentColor.b - (short)closestMatch.b;

        gd->setPx((word)i, (byte)y, (byte) cidx);

        // pixels[x + 1][y    ] := pixels[x + 1][y    ] + quant_error × 7 / 16
        if (i + 1 < 320) {
            screenPixel = applyError(co[i + 1], quantError, 0.4375);
            gd->setPx((word) i + 1, (byte) y, (byte) findNearestColor(screenPixel, 0));

            // char buf[32]; memset(buf, 0, 32);
            // sprintf(buf, " %f %f %f - %f %f %f",currentColor.r, currentColor.g, currentColor.b, screenPixel.r, screenPixel.g, screenPixel.b);
            // gd->printText(1, 10, buf, 5, 0);
            // sprintf(buf, "%f %f %f", closestMatch.r, closestMatch.g, closestMatch.b);
            // gd->printText(1, 11, buf, 5, 0);

            co[i + 1].r = screenPixel.r;
            co[i + 1].g = screenPixel.g;
            co[i + 1].b = screenPixel.b;
        }

        // pixels[x - 1][y + 1] := pixels[x - 1][y + 1] + quant_error × 3 / 16
        if (i > 1) {
            screenPixel = applyError(co[320 + i - 1], quantError, 0.1875);
            gd->setPx((word) i - 1, (byte) y + 1, (byte) findNearestColor(screenPixel, 0));
            co[320 + i - 1].r = screenPixel.r;
            co[320 + i - 1].g = screenPixel.g;
            co[320 + i - 1].b = screenPixel.b;
        }

        // pixels[x    ][y + 1] := pixels[x    ][y + 1] + quant_error × 5 / 16
        screenPixel = applyError(co[320 + i], quantError, 0.3125);
        gd->setPx((word) x, (byte)  y + 1, (byte)  findNearestColor(screenPixel, 0));
        co[320 + i].r = screenPixel.r;
        co[320 + i].g = screenPixel.g;
        co[320 + i].b = screenPixel.b;

        // pixels[x + 1][y + 1] := pixels[x + 1][y + 1] + quant_error × 1 / 16
        if (i + 1 < 320) {
            screenPixel = applyError(co[320 + i + 1], quantError, 0.0625);
            gd->setPx((word) x + 1, (byte)  y + 1, (byte) findNearestColor(screenPixel, 0));
            co[320 + i + 1].r = screenPixel.r;
            co[320 + i + 1].g = screenPixel.g;
            co[320 + i + 1].b = screenPixel.b;
        }
    }
}