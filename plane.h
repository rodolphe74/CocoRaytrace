//plane intersect prototype
#include "ray.h"

typedef struct intersect{
   PV_t p;
   FLOATING plnT;
   int flag;
}xpnt_t;

xpnt_t plane_intersect(ray_t *, plane_t *);
