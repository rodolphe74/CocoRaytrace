#ifdef CMOC_COMPILER
#include <coco.h>
#define pow(x,n)(expf(n*logf(x)))
#else
#include <stdio.h>
#include <math.h>
#endif

// exported floating coco float functions
// float sinf(float radians);
// float cosf(float radians);
// float tanf(float radians);  // argument must not be pi/2 or 3*pi/2 or any other equivalent angle
// float atanf(float radians);  // returns value in [-pi/2, pi/2]
// float logf(float x);  // x > 0
// float expf(float x);
// float sqrtf(float x);  // x >= 0
// float fabsf(float radians);
// float floorf(float x);  // returns the largest integral value that is not greater than x
// float truncf(float x);  // returns x rounded to the nearest integer value that is not larger in magnitude than x
// float roundf(float x);  // returns x rounded to the nearest integer, but halfway cases are rounded away from zero

int main()
{
    float a = 1.2;
    float b = 2.3;
    float c = a + b;
    printf("%f + %f = %f\n", a, b, c);
    printf("sqrt(%f) = %f\n", c, sqrtf(c));
    printf("pow(%f,%f) = %f\n", c, a, pow(c, a));
    return 0;
}