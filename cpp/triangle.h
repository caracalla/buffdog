#ifndef BUFFDOG_TRIANGLE
#define BUFFDOG_TRIANGLE

#include "line.h"

// Logic for drawing 2D triangles

struct Point {
	int x;
	int y;
};

void drawLineFromPoints(Point start, Point end, int color) {
	drawLine(start.x, start.y, end.x, end.y, color);
}

// mostly for debugging
void drawPoint(Point p, int color) {
	for (int i = p.x - 2; i < p.x + 2; i++) {
		for (int j = p.y - 2; j < p.y + 2; j++) {
			device::setPixel(i, j, color);
		}
	}
}

struct Triangle2D {
	Point p0;
	Point p1;
	Point p2;
	int color;

	void draw() {
		drawLineFromPoints(this->p0, this->p1, this->color);
		drawLineFromPoints(this->p1, this->p2, this->color);
		drawLineFromPoints(this->p2, this->p0, this->color);
	}

	void fill() {
		// sort from highest to lowest (?)
		Point temp;
		if (this->p0.y > this->p1.y) {
			temp = this->p0;
			this->p0 = this->p1;
			this->p1 = temp;
		}

		if (this->p0.y > this->p2.y) {
			temp = this->p0;
			this->p0 = this->p2;
			this->p2 = temp;
		}

		if (this->p1.y > this->p2.y) {
			temp = this->p2;
			this->p2 = this->p1;
			this->p1 = temp;
		}

		// p0 to p2 is long side
		// if p2.x > p1.x
		//   long side is right
		// else
		//   long side is left (or, they're equal, then it doesn't matter?)
		//
		// x = (int)(x_0 + (y - y_0) * dx / (float)dy)

		int dx01 = this->p1.x - this->p0.x;
		double dy01 = this->p1.y - this->p0.y;
		if (dy01 == 0) { dy01 = 0.000001; }
		double m01 = dx01 / dy01;

		int dx12 = this->p2.x - this->p1.x;
		double dy12 = this->p2.y - this->p1.y;
		if (dy12 == 0) { dy12 = 0.000001; }
		double m12 = dx12 / dy12;

		int dx02 = this->p2.x - this->p0.x;
		double dy02 = this->p2.y - this->p0.y;
		if (dy02 == 0) { dy02 = 0.000001; }
		double m02 = dx02 / dy02;

		int start_x;
		int end_x;

		// this is slow
		for (int y = this->p0.y; y < this->p1.y; y++) {
			int x01 = (int)(this->p0.x + (y - this->p0.y) * m01);
			int x02 = (int)(this->p0.x + (y - this->p0.y) * m02);

			if (x01 < x02) {
				start_x = x01;
				end_x = x02;
			} else {
				start_x = x02;
				end_x = x01;
			}

			drawLine(start_x, y, end_x, y, this->color);
		}

		for (int y = this->p1.y; y <= this->p2.y; y++) {
			int x12 = (int)(this->p1.x + (y - this->p1.y) * m12);
			int x02 = (int)(this->p0.x + (y - this->p0.y) * m02);

			if (x12 < x02) {
				start_x = x12;
				end_x = x02;
			} else {
				start_x = x02;
				end_x = x12;
			}

			drawLine(start_x, y, end_x, y, this->color);
		}
	}
};

typedef struct {
	Point p0;
	Point p1;
	Point p2;
	int color;
} tri2d;

#endif
