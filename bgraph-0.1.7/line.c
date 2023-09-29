// Public domain source (as of 2018-09-02):
// https://github.com/miloyip/line/blob/master/line_bresenham.c

#include "Primitive.h"


#define ABS_(a) ((absval = (int) (a)) < 0 ? (-absval) : (absval))


void
Primitive_line(word x0, byte y0, word x1, byte y1, byte color,
               Primitive_setPixelFuncPtr setPixel, void *cookie)
{
    int dx, dy, err, e2;
    char sx, sy;

#if 0
    int absval;
    dx = ABS_(x1 - x0);
    sx = x0 < x1 ? 1 : -1;
    dy = ABS_((int) y1 - (int) y0);
    sy = y0 < y1 ? 1 : -1;
    err = (dx > dy ? dx : -dy) / 2;
#else
    asm
    {
        ; Compute dx = |x1 - x0| and sx = x0 < x1 ? 1 : -1.
        ldd     :x1
        subd    :x0
        bgt     @x1_gt_x0       ; signed comparison
        ; x1 <= x0, so x1 - x0 <= 0, so abs. value is negation of D.
        coma
        comb
        addd    #1
        std     :dx
        ldb     #-1
        stb     :sx
        bra     @compute_dy_sy
@x1_gt_x0
        ; x1 > x0.
        std     :dx
        ldb     #1
        stb     :sx
;
@compute_dy_sy
        ; Compute dy = |(int) y1 - (int) y0| and sy = y0 < y1 ? 1 : -1.
        ; (int) y1 - (int) y0
        ldb     :y0
        clra
        std     :err        ; use 'err' as temp storage
        ldb     :y1
        clra
        subd    :err
        bgt     @y1_gt_y0
        ; y1 <= y0
        coma
        comb
        addd    #1
        std     :dy
        ldb     #-1
        stb     :sy
        bra     @compute_err
@y1_gt_y0
        ; y1 > y0.
        std     :dy
        ldb     #1
        stb     :sy
;
@compute_err
        ; Compute err = (dx > dy ? dx : -dy) / 2.
        ldd     :dx
        cmpd    :dy
        bgt     @divDBy2
        ldd     :dy
        asra
        rorb                ; D = dy / 2
        coma
        comb
        addd    #1          ; D = -dy / 2
        bra     @storeD
@divDBy2
        asra                ; shift signed word right 1 bit
        rorb
@storeD
        std     :err
    }
#endif

#if 0
    while (setPixel(x0, y0, color, cookie), x0 != x1 || y0 != y1)
    {
        int e2 = err;
        if (e2 + dx > 0)  // if e2 > -dx
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 - dy < 0)  // if e2 < dy
        {
            err += dx;
            y0 += sy;
        }
    }
#else
    asm
    {
        bra     @whileCond
@whileBody
        ; 1st if().
        ldd     :err
        std     :e2
        addd    :dx
        ble     @firstIfFails
        ; e2 > -dx.
        ldd     :err
        subd    :dy
        std     :err
        ldb     :sx
        sex
        addd    :x0
        std     :x0
@firstIfFails
        ; 2nd if().
        ldd     :e2
        subd    :dy
        bge     @secondIfFails
        ; e2 < dy.
        ldd     :err
        addd    :dx
        std     :err
        ldb     :y0
        addb    :sy
        stb     :y0
@secondIfFails
@whileCond
        ldx     :cookie
        ldb     :color
        clra                ; C rule: promote to unsigned int
        pshs    x,d
        ldb     :y0
        pshs    d           ; A is still 0
        ldd     :x0
        pshs    d
        ldx     :setPixel
        jsr     ,x
        leas    4*2,s
        ldd     :x0
        cmpd    :x1
        bne     @whileBody
        ldb     :y0
        cmpb    :y1
        bne     @whileBody
    }
#endif
}
