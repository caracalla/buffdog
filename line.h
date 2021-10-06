#ifndef BUFFDOG_LINE
#define BUFFDOG_LINE

#include "point.h"


#define ROUND(a) ((int)(a + 0.5))


// void drawLine(int x1, int y1, int x2, int y2, int color);
void drawLine(Point start, Point end, int color);

#endif
