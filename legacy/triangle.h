#ifndef BUFFDOG_TRIANGLE
#define BUFFDOG_TRIANGLE

#include "line.h"
#include "vec3.h"

// Logic for drawing 2D triangles

typedef struct {
	int x;
	int y;
} point;

typedef struct {
	point p0;
	point p1;
	point p2;
	int color;
} triangle;

void drawLine(point start, point end, int color) {
	draw_line(start.x, start.y, end.x, end.y, color);
}

void drawTriangle(triangle tri) {
	drawLine(tri.p0, tri.p1, tri.color);
	drawLine(tri.p1, tri.p2, tri.color);
	drawLine(tri.p2, tri.p0, tri.color);
}

void fillTriangle(triangle tri) {
	// sort from highest to lowest (?)
	point temp;
	if (tri.p0.y > tri.p1.y) {
		temp = tri.p0;
		tri.p0 = tri.p1;
		tri.p1 = temp;
	}

	if (tri.p0.y > tri.p2.y) {
		temp = tri.p0;
		tri.p0 = tri.p2;
		tri.p2 = temp;
	}

	if (tri.p1.y > tri.p2.y) {
		temp = tri.p2;
		tri.p2 = tri.p1;
		tri.p1 = temp;
	}

	// p0 to p2 is long side
	// if p2.x > p1.x
	//   long side is right
	// else
	//   long side is left (or, they're equal, then it doesn't matter?)
	//
	// x = (int)(x_0 + (y - y_0) * dx / (float)dy)

	int dx01 = tri.p1.x - tri.p0.x;
	double dy01 = tri.p1.y - tri.p0.y;
	if (dy01 == 0) { dy01 = 0.000001; }
	double m01 = dx01 / dy01;

	int dx12 = tri.p2.x - tri.p1.x;
	double dy12 = tri.p2.y - tri.p1.y;
	if (dy12 == 0) { dy12 = 0.000001; }
	double m12 = dx12 / dy12;

	int dx02 = tri.p2.x - tri.p0.x;
	double dy02 = tri.p2.y - tri.p0.y;
	if (dy02 == 0) { dy02 = 0.000001; }
	double m02 = dx02 / dy02;

	int start_x;
	int end_x;

	// this is slow
	for (int y = tri.p0.y; y < tri.p1.y; y++) {
		int x01 = (int)(tri.p0.x + (y - tri.p0.y) * m01);
		int x02 = (int)(tri.p0.x + (y - tri.p0.y) * m02);

		if (x01 < x02) {
			start_x = x01;
			end_x = x02;
		} else {
			start_x = x02;
			end_x = x01;
		}

		draw_line(start_x, y, end_x, y, tri.color);
	}

	for (int y = tri.p1.y; y <= tri.p2.y; y++) {
		int x12 = (int)(tri.p1.x + (y - tri.p1.y) * m12);
		int x02 = (int)(tri.p0.x + (y - tri.p0.y) * m02);

		if (x12 < x02) {
			start_x = x12;
			end_x = x02;
		} else {
			start_x = x02;
			end_x = x12;
		}

		draw_line(start_x, y, end_x, y, tri.color);
	}
}

#endif
