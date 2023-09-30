
#include "type.h"
#include "plane.h"

#ifndef CMOC_COMPILER
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#else
#include <coco.h>
#define pow(x,n)(expf(n*logf(x)))
#define sqrt(x)(sqrtf(x))
#endif

//ray-plane intersect function
xpnt_t plane_intersect(ray_t *ray, plane_t *pln)
{
    xpnt_t ix_point;
    FLOATING pdnom = vec_dot(&pln->norm, &ray->dir);

    if(pdnom <= 0) {
        ix_point.flag = 0;
    }

    FLOATING pnumr = pln->D - vec_dot(&(pln->norm), &(ray-> or ));
    FLOATING planeT = pnumr / pdnom;

    if(planeT > 0) {
        ix_point.p.x = ray->or.x + ray->dir.x * planeT;
        ix_point.p.y = ray->or.y + ray->dir.y * planeT;
        ix_point.p.z = ray->or.z + ray->dir.z * planeT;
        ix_point.plnT = planeT;
        ix_point.flag = 1;
    }

    return ix_point;

}
