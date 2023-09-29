#ifndef ERRDIFF_H
#define ERRDIFF_H
#include "type.h"
#include "bgraph-0.1.7/cc4graph.h"
#include "ray.h"

int findNearestColor(color_t c, char multiply);
void errorDiffusiongDither(GraphicsDriver *gd, word x, byte y, color_t *co);

#endif