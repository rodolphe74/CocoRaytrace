//light.c light functions for plane and sphere

#include "light.h"

#ifndef CMOC_COMPILER
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#else
#include <coco.h>
#define pow(x,n)(expf(n*logf(x)))
#define sqrt(x)(sqrtf(x))
#endif

//function for plane lighting
color_t pln_lighting(light_t *light, ray_t *ray, xpnt_t *plnx)
{
    FLOATING T = plnx->plnT;
    color_t RGB;
    RGB.r = 0;
    RGB.g = 0;
    RGB.b = 0;

    //add ambient lighting value to plane
    FLOATING amb = 0.1;
    RGB.r += amb;
    RGB.g += amb;
    RGB.b += amb;

    //viewpoint vector V
    PV_t V = ray->dir;

    //vectors for normal, N
    PV_t normv = vec_scale(T, &V);
    PV_t normorg = vec_sum(&ray-> or, &normv);

    /*normalized vector N (difference
    of plane point and norm origin)*/
    PV_t N = vec_diff(&normorg, &plnx->p);

    //normalize the Normal
    FLOATING tp = vec_dot(&N, &N);
    FLOATING tmp = 1.0 / sqrt(tp);
    N = vec_scale(tmp, &N);

    //vector for light L
    PV_t d = vec_diff(&light->origin, &normorg);

    //length of the distance from the origin of the plane
    FLOATING dL = vec_len(&d);

    //constants for attenuated lighting
    FLOATING c1 = .002;
    FLOATING c2 = .02;
    FLOATING c3 = .2;

    //attenuated lighting, calculated and added to rgb
    FLOATING atten = 1.0 / ((c1 * (dL * dL)) + (c2 * dL) + c3);
    if(atten < 0.1) {
        atten = 0.1;
    }
    RGB.r += atten;
    RGB.g += atten;
    RGB.b += atten;

    /*return final RGB for
    intersected plane pixel*/
    return RGB;

}

//function for sphere lighting
color_t do_lighting(light_t *light, ray_t *ray, sphere_t *sph, FLOATING *t)
{
    FLOATING T = *t;
    color_t RGB;
    RGB.r = 0;
    RGB.g = 0;
    RGB.b = 0;

    //add ambient lighting value to rgb
    FLOATING amb = 0.1;
    RGB.r += amb;
    RGB.g += amb;
    RGB.b += amb;

    //viewpoint vector V
    PV_t V = ray->dir;

    //normalize V
    FLOATING tpr = vec_dot(&V, &V);
    FLOATING tmpr = 1.0 / sqrt(tpr);
    V = vec_scale(tmpr, &V);

    //vectors for normal, N
    PV_t normv = vec_scale(T, &V);
    PV_t normorg = vec_sum(&ray-> or, &normv);

    /*normalized vector N (difference
    of sphere center and norm origin)*/
    PV_t N = vec_diff(&normorg, &sph->ctr);

    //normalize the Normal
    FLOATING tp = vec_dot(&N, &N);
    FLOATING tmp = 1.0 / sqrt(tp);
    N = vec_scale(tmp, &N);

    //vector for light L
    PV_t litlen = vec_diff(&light->origin, &normorg);
    PV_t L = vec_scale(1.0 / T, &litlen);

    /* diffuse lighting:
    cos theta = N dot L; angle between light and normal vectors
    multiplied by the current light amounts, added to rgb*/
    FLOATING cos_theta = vec_dot(&L, &N);
    if(cos_theta > 0) {
        RGB.r += cos_theta * light->amount.r * (sph->sclr.r);
        RGB.g += cos_theta * light->amount.g * (sph->sclr.g);
        RGB.b += cos_theta * light->amount.b * (sph->sclr.b);
    }

    /* equation for R = L - 2 * (N dot L) * N
    R is the vector between N and V
    broken down into working components via temp variables*/
    PV_t tempr = (vec_scale((2 * vec_dot(&L, &N)), &N));
    PV_t R = vec_diff(&L, &tempr);
    FLOATING tmp2 = vec_dot(&R, &R);
    tmp2 = 1.0 / (sqrt(tmp2));
    R = vec_scale(tmp2, &R);

    /* specular lighting
     cosine alpha = dot product of R and V
     add (cosine alpha^40) to rgb */
    FLOATING cos_alpha = vec_dot(&R, &V);
    FLOATING n = 40.0 / cos_alpha;
    if((cos_alpha > 0) && (cos_theta > 0)) {
        RGB.r += pow(cos_alpha, n);
        RGB.g += pow(cos_alpha, n);
        RGB.b += pow(cos_alpha, n);
    }





    return RGB;
}

