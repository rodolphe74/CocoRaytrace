//ray.c

#ifndef CMOC_COMPILER
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#else
#include <coco.h>
#define pow(x,n)(expf(n*logf(x)))
#define sqrt(x)(sqrtf(x))
#define floor(x)(floorf(x))
#endif

#define TRACE_EVERY 1

#include "type.h"
#include "light.h"
#include "errdiff.h"

// unsigned int counter = 0;

// color_t palette[] = {
// 	{2,2,2},
// 	{74,2,0},
// 	{74,74,172},
// 	{74,74,74},
// 	{172,74,2},
// 	{74,172,2},
// 	{253,74,2},
// 	{172,172,172},
// 	{2,172,253},
// 	{235,172,74},
// 	{74,172,172},
// 	{74,253,74},
// 	{253,172,2},
// 	{2,253,253},
// 	{172,172,2},
// 	{253,253,253}
// };

// int findNearestColor(color_t c)
// {
// 	int minIndex = 0;
// 	FLOATING minDistance = 400000.0;
// 	FLOATING distance = 0;
// 	for (unsigned char i = 0; i < 16; i++) {
// 		distance = (
// 			(c.r - palette[i].r) * (c.r - palette[i].r) +
// 			(c.g - palette[i].g) * (c.g - palette[i].g) + 
// 			(c.b - palette[i].r) * (c.b - palette[i].b) /*^0.5*/
// 		);
// 		if (distance < minDistance) {
// 			minIndex = i;
// 			minDistance = distance;
// 		}
// 	}
// 	return minIndex;
// }

color_t twoLines[640];
word pixelCounter = 0;
byte startAtLine = 0;

#ifndef CMOC_COMPILER
void makeppm(char *file, unsigned char *ppm, int wd, int ht)
{
    /*file pointer set to the output file to write (,"w")*/
    FILE *fp;
    fp = fopen("test1.ppm", "w");

    /*print the ppm file header*/
    fprintf(fp, "P6 %d %d %d\n",  wd, ht, 255);

    /* from data ppm[] holds, 3 (size of pixel element),
    size of ppm[], to fp (the file pointer) */
    fwrite(ppm, 3, wd*ht, fp);

    /*close the file*/
    fclose(fp);
}
#endif


// int main()
int ray(GraphicsDriver *gd)
{

	// show current palette
	// for (int i = 0; i < 16; i++) {
    //     // gd->filledRectangle(i * 16, 0, i * 16 + 16, 16, i);

    //     gd->setPx(i*16, 10, i);
    // }
	// return;

    

    ray_t ray;

    //pixel dimensions for the output resolution of ppm image
    int wid = 320;
    int hgt = 200;
    FLOATING ratio = 480.0/400.0;

    //allocate memory for spheres, initialize sphere data
    // sphere_t *sph;
    // int num = 2;
    // sph = (sphere_t*)malloc(sizeof(sphere_t) * num);
	sphere_t sph[2];
	int num = 2;

    // printf("sizeof sphere:%d\n", sizeof(sphere_t) * num);

    sph[0].ctr.x = 0.5;
    sph[0].ctr.y = 0.8;
    sph[0].ctr.z = 4.0;
    sph[0].radius = 0.5;
    sph[0].sclr.r = 0.3;
    sph[0].sclr.g = 0.0;
    sph[0].sclr.b = 0.0;


    sph[1].ctr.x = -0.5;
    sph[1].ctr.y = 0.15;
    sph[1].ctr.z = 4.2;
    sph[1].radius = 0.6;
    sph[1].sclr.r = 0.0;
    sph[1].sclr.g = 0.3;
    sph[1].sclr.b = 0.0;

    //initiate the plane
    plane_t pln;
    pln.norm.x = 0.0;
    pln.norm.y = 1.0;
    pln.norm.z = 0.0;
    pln.D = -1.0;


    //light origin and color levels
    light_t light;
    light.origin.x = 5.0;
    light.origin.y = 10.0;
    light.origin.z = -2.0;
    light.amount.r = 1.0;
    light.amount.g = 1.0;
    light.amount.b = 1.0;


    //unsigned char array, height * width * 3(for r,g,b)
	#ifndef CMOC_COMPILER
    unsigned char ppm[3*wid*hgt];
	#endif

    //loop through columns and rows of the image
    int x=0, y=0, lasty=1;
    for(y=0; y<hgt; y++) {
        for(x=0; x<wid; x++) {

            //initialize origin and direction of ray
            ray.or.x = 0;
            ray.or.y = 0;
            ray.or.z = 0;

            //    ray.dir.x = -0.67 + x/480.0;
            //    ray.dir.y = 0.5 - y/480.0;
            ray.dir.x = -0.67 + x/ (float)wid;
            ray.dir.y = 0.5 - y/ ((float)wid / ratio);
            ray.dir.z = 1.0;

            //initialize the 0.0 - 1.0 background color
            color_t rgb;
            rgb.r = 0.1;
            rgb.g = 0.3;
            rgb.b = 0.5;

            //initialize t to a large number
            FLOATING t = 20000.0;

            /*check the value returned by plane_intersect()
            	to determine hit or miss*/

            //plane intersect flag
            xpnt_t plnx = plane_intersect(&ray, &pln);

            //if plane intersection happens, floor function on x,y,z
            //then bitwise function to determine checkerboard
            if(plnx.flag == 1) {
                if((int)floor(plnx.p.x) + (int)floor(plnx.p.y) + (int)floor(plnx.p.z) & 1) {
                    rgb.r = 0.0;
                    rgb.g = 0.0;
                    rgb.b = 0.0;
                } else {
                    rgb.r = 1.0;
                    rgb.g = 1.0;
                    rgb.b = 1.0;
                }
                color_t RGB1 = pln_lighting(&light, &ray, &plnx);
                rgb.r *= RGB1.r;
                rgb.g *= RGB1.g;
                rgb.b *= RGB1.b;
            }

            //loop through spheres to determine closest t
            int i;
            int closest_i = -1;
            for(i=0; i<2; i++) {


                if(intersect(&ray, &sph[i], &t, x, y) == 1) {
                    closest_i = i;
                }

                //process lighting on pixel for the closest t
                if(closest_i >= 0) {
                    color_t RGB2 = do_lighting(&light, &ray, &sph[closest_i], &t);
                    rgb.r = sph[closest_i].sclr.r + RGB2.r;
                    rgb.g = sph[closest_i].sclr.g + RGB2.g;
                    rgb.b = sph[closest_i].sclr.b + RGB2.b;
                }

            }

            //while sphere intersect returns a hit, process rgb values
            //while(intersect(&ray, &sph, &t) == 1);
            //capping rgb at or below 1.0
            if(rgb.r > 1.0) rgb.r = 1.0;
            if(rgb.g > 1.0) rgb.g = 1.0;
            if(rgb.b > 1.0) rgb.b = 1.0;

            //(x+y*width)*3 = current pixel * rgb then
            //+0, +1 or +2 to access correct color in pixel.
            //rgb factor for each pixel multiplied by max color value 255
			#ifndef CMOC_COMPILER
            ppm[(x + y*wid)*3 + 0] = (unsigned char)(rgb.r*255.0);
            ppm[(x + y*wid)*3 + 1] = (unsigned char)(rgb.g*255.0);
            ppm[(x + y*wid)*3 + 2] = (unsigned char)(rgb.b*255.0);
			#else
			
			#endif
			// if ( (unsigned char)(rgb.r*255.0) > 128)
			// 	gd->line(x, y, x, y, 1);
			// else
			// 	gd->line(x, y, x, y, 2);

            // no ed
			color_t co = {(rgb.r*255.0), (rgb.g*255.0), (rgb.b*255.0)};
			// gd->line(x, y, x, y, findNearestColor(co, 0));

            // ed
            gd->setPx(x, y, 6);

            twoLines[pixelCounter++] = co;
            // twoLines[pixelCounter++] = rgb;
            // if (pixelCounter == 320) {
            //     // move up last line
            //     memcpy(twoLines, twoLines + 320 * sizeof(color_t), 320 * sizeof(color_t));
            //     // one line
            //     errorDiffusiongDither(gd, 0, y - 1, twoLines);
            // }
            if (pixelCounter >= 640) {
                // two lines
                errorDiffusiongDither(gd, 0, y - 1, twoLines);
                pixelCounter = 320;
                // memcpy(twoLines, twoLines + 320 * sizeof(color_t), 320 * sizeof(color_t));
                for (int i = 320; i < 640; i++) {
                    twoLines[i - 320] = twoLines[i];
                }

            }
        }
    }
//output the ppm, and free memory used for sphere
#ifndef CMOC_COMPILER
    makeppm("test1.ppm", ppm, wid, hgt);
#endif
    // free(sph);
}


