#ifdef CMOC_COMPILER
#include <coco.h>
#define pow(x,n)(expf(n*logf(x)))
#else
#include <stdio.h>
#include <math.h>
#endif

typedef float FLOATING;

typedef struct color_type{
   FLOATING r;
   FLOATING g;
   FLOATING b;
} color_t;

color_t palette[] = {
	{2,2,2},
	{74,2,0},
	{74,74,172},
	{74,74,74},
	{172,74,2},
	{74,172,2},
	{253,74,2},
	{172,172,172},
	{2,172,253},
	{235,172,74},
	{74,172,172},
	{74,253,74},
	{253,172,2},
	{2,253,253},
	{172,172,2},
	{253,253,253}
};

int findNearestColor(color_t c)
{
	int minIndex = 0;
	FLOATING minDistance = 400000.0;
	FLOATING distance = 0;
	for (unsigned char i = 0; i < 16; i++) {
		distance = (
			(c.r - palette[i].r) * (c.r - palette[i].r) +
			(c.g - palette[i].g) * (c.g - palette[i].g) + 
			(c.b - palette[i].r) * (c.b - palette[i].b) /*^0.5*/
		);
		if (distance < minDistance) {
			minIndex = i;
			minDistance = distance;
		}
	}
	return minIndex;
}


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

    // color_t co = {74,74,125};
    color_t co = {250,250,250};
    printf("%d\n", findNearestColor(co));
    return 0;
}