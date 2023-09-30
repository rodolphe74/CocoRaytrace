//vector.h

#include "type.h"

#ifndef CMOC_COMPILER
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#else
#include <coco.h>
#define pow(x,n)(expf(n*logf(x)))
#define sqrt(x)(sqrtf(x))
#endif

//vector structure
typedef struct vector_type {
    FLOATING x, y, z;
} PV_t;


//  prototypes for the vector functions


PV_t vec_sum(PV_t *, PV_t *);
PV_t vec_diff(PV_t *, PV_t *);
FLOATING vec_dot(PV_t *, PV_t *);
PV_t vec_scale(FLOATING fact, PV_t *);
FLOATING vec_len(PV_t *);
