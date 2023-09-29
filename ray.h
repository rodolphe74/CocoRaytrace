//ray.h containing structure types
#ifndef RAY_H
#define RAY_H

#include "vector.h"
#include "bgraph-0.1.7/cc4graph.h"

//ray structure w/origin and direction points/vectors
typedef struct ray_type{
   PV_t or;
   PV_t dir;
} ray_t;

//color structure w/red green blue values
typedef struct color_type{
   FLOATING r;
   FLOATING g;
   FLOATING b;
} color_t;

//struct for plane
typedef struct plane_type{
   PV_t norm;
   FLOATING D;
} plane_t;

//sphere structure w/center point and radius
typedef struct sphere_type{
   PV_t ctr;
   FLOATING radius;
   color_t sclr; 
} sphere_t;
	
//light structure w/origin point and color amount
typedef struct light_type{
   PV_t origin;
   color_t amount;
} light_t;


int ray(GraphicsDriver *gd);

#endif