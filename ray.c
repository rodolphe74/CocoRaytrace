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
#ifdef DCMOC_COMPILER
#include "errdiff.h"
#endif

// Two lines RGB buffer used to dither before displaying to frame buffer
#ifdef CMOC_COMPILER
color_t twoLines[640];
word pixelCounter = 0;
byte startAtLine = 0;
#endif

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
    fwrite(ppm, 3, wd * ht, fp);

    /*close the file*/
    fclose(fp);
}
#endif


#ifndef CMOC_COMPILER
int main()
#else
int ray(GraphicsDriver *gd)
#endif
{
    ray_t ray;

    //pixel dimensions for the output resolution of ppm image
#ifdef CMOC_COMPILER
    int wid = 320;
    int hgt = 200;
    FLOATING ratio = 480.0 / 400.0;
#else
    int wid = 320;
    int hgt = 240;
    FLOATING ratio = 1.0;
#endif

    // 2 spheres
    sphere_t sph[2];
    int num = 2;

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

#ifndef CMOC_COMPILER
    //unsigned char array, height * width * 3(for r,g,b)
    unsigned char ppm[3 * wid * hgt];
#endif

    //loop through columns and rows of the image
    int x = 0, y = 0, lasty = 1;
    for(y = 0; y < hgt; y++) {
        for(x = 0; x < wid; x++) {

            //initialize origin and direction of ray
            ray.or.x = 0;
            ray.or.y = 0;
            ray.or.z = 0;

            //    ray.dir.x = -0.67 + x/480.0;
            //    ray.dir.y = 0.5 - y/480.0;
            ray.dir.x = -0.67 + x / (float)wid;
            ray.dir.y = 0.5 - y / ((float)wid / ratio);
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
            for(i = 0; i < 2; i++) {

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
            ppm[(x + y * wid) * 3 + 0] = (unsigned char)(rgb.r * 255.0);
            ppm[(x + y * wid) * 3 + 1] = (unsigned char)(rgb.g * 255.0);
            ppm[(x + y * wid) * 3 + 2] = (unsigned char)(rgb.b * 255.0);
#else
            // current RGB pixel color
            color_t co = {(rgb.r * 255.0), (rgb.g * 255.0), (rgb.b * 255.0) };

            // to display undithered, uncomment next line
            // gd->line(x, y, x, y, findNearestColor(co, 0));

            // show current computed pixel
            gd->setPx((word)x, (byte)y, 6);

            // feed dithering 2 lines buffer (need at least 2 lines to apply Floyd Steinberg)
            twoLines[pixelCounter++] = co;
            if (pixelCounter >= 640) {
                // two lines
                errorDiffusiongDither(gd, 0, y - 1, twoLines);

                // move last line upstairs (will apply 1st line fs matrix on going up second line on next iteration)
                pixelCounter = 320;
                for (int i = 320; i < 640; i++) {
                    twoLines[i - 320] = twoLines[i];
                }
            }
#endif
        }

    }


#ifndef CMOC_COMPILER
    //output the ppm, and free memory used for sphere
    makeppm("test1.ppm", ppm, wid, hgt);
#endif
    return 0;
}


