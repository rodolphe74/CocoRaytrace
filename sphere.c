//sphere.c, sphere intersect function

#include "type.h"
#include "sphere.h"


#ifndef CMOC_COMPILER
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#else
#include <coco.h>
#define pow(x,n)(expf(n*logf(x)))
#define sqrt(x)(sqrtf(x))
#endif


// ray/sphere intersect function
// int intersect(ray_t *ray, sphere_t *sph, FLOATING *t)
// {
//     printf("intersect:%f\n", *t);

//     int hit = 0;
//     //calculate A, B, and C variables to determine the discriminant
//     FLOATING A = vec_dot(&ray->dir, &ray->dir);
//     PV_t dist = vec_diff(&ray->or, &sph->ctr);
//     FLOATING B = 2 * vec_dot(&ray->dir, &dist);
//     FLOATING C = vec_dot(&dist, &dist) - (sph->radius * sph->radius);
//     //equation for the discriminant B^2 - 4AC
//     FLOATING discriminant = (B * B) - 4 * (A * C);

// 	printf("a:%f b:%f c:%f  disc:%f\n", A, B, C, discriminant);

//     //return no hit if disc < 0
//     if(discriminant < 0)
//         hit = 0;
// 	// } else {
//     //determining the t0 and t1 for the sphere intersection
//     FLOATING discsqrt = sqrt(discriminant);


//     FLOATING t0 = (-B - discsqrt)/(2);
//     FLOATING t1 = (-B + discsqrt)/(2);
// 	printf("discsqrt:%f %f %f\n", discsqrt, t0, t1);

//     //set t0 to not greater (closer) than t1
//     if(t0<0 && t1==0) {
// 		printf("1\n");
//         hit=0;
//     }
//     if(t0 < 0.0) {
// 		printf("2\n");
//         *t = t1;
//     }

//     //if its also greater than 0, return 1 for hit. If not then 0
//     if(t0 > 0.0) {
// 		printf("3\n");
//         *t = t0;
//         hit = 1;
//     } else
// 		printf("4 NAN\n");
//         hit = 0;

// 	// }
//     return hit;
// }

// ray/sphere intersect function
int intersect(ray_t *ray, sphere_t *sph, FLOATING *t, int x, int y)
{
    int hit = 0;
    //calculate A, B, and C variables to determine the discriminant
    FLOATING A = vec_dot(&ray->dir, &ray->dir);
    PV_t dist = vec_diff(&ray-> or, &sph->ctr);
    FLOATING B = 2 * vec_dot(&ray->dir, &dist);
    FLOATING C = vec_dot(&dist, &dist) - (sph->radius * sph->radius);
    //equation for the discriminant B^2 - 4AC
    // FLOATING discriminant = (B * B) - 4 * (A * C);
    FLOATING discriminant = (B * B) - 4.0f * (A * C);
// if(y == 22)
// 	printf("(%d,%d) discsqrt:%f %f %f\n", x, y, A, B, C);

    //return no hit if disc < 0
    if(discriminant < 0) {
        hit = 0;
        return hit;
    }

    //determining the t0 and t1 for the sphere intersection
    FLOATING discsqrt = sqrt(discriminant);
    FLOATING t0 = (-B - discsqrt) / (2);
    FLOATING t1 = (-B + discsqrt) / (2);

    // printf("(%d,%d)  discsqrt:%f %f %f\n", x,y,discsqrt, t0, t1);
    //set t0 to not greater (closer) than t1
    if(t0 < 0 && t1 == 0) {
        hit = 0;
    }
    if(t0 < 0.0) {
        *t = t1;
    }

    //if its also greater than 0, return 1 for hit. If not then 0
    if(t0 > 0.0) {
        *t = t0;
        hit = 1;
    } else
        hit = 0;


    return hit;
}