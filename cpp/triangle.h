#ifndef BUFFDOG_TRIANGLE
#define BUFFDOG_TRIANGLE

#include "device.h"
#include "line.h"
#include "vector.h"

#define THREE_MIN(x0, x1, x2) x0 < x1 ? (x2 < x0 ? x2 : x0) : (x2 < x1 ? x2 : x1)
#define THREE_MAX(x0, x1, x2) x0 > x1 ? (x2 > x0 ? x2 : x0) : (x2 > x1 ? x2 : x1)

// Logic for drawing 2D triangles

struct Point {
	int x;
	int y;
};

void drawLineFromPoints(Point start, Point end, int color) {
	drawLine(start.x, start.y, end.x, end.y, color);
}

int colorFromVector(Vector vec) {
	return device::color(vec.x, vec.y, vec.z);
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
	Vector color;

	void draw() {
		int color = colorFromVector(this->color);
		drawLineFromPoints(this->p0, this->p1, color);
		drawLineFromPoints(this->p1, this->p2, color);
		drawLineFromPoints(this->p2, this->p0, color);
	}

	void fill() {
		int color = colorFromVector(this->color);

		// sort from highest (p2) to lowest (p0)
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

		// po to p2 is the long vertical side
		double dy02 = this->p2.y - this->p0.y;

		if (dy02 == 0) {
			// the whole thing is flat horizontally
			// draw a horizontal line from the min x to the max x
			int min_x = THREE_MIN(this->p0.x, this->p1.x, this->p2.x);
			int max_x = THREE_MAX(this->p0.x, this->p1.x, this->p2.x);

			drawHorizontalLine(this->p2.y, min_x, max_x, color);
			return;
		}

		int dx02 = this->p2.x - this->p0.x;
		double m02 = dx02 / dy02;
		// the x value for line p2 - p0 changes constantly over the whole triangle
		double x02 = this->p0.x;

		// p0 to p1 is the bottom half
		double dy01 = this->p1.y - this->p0.y;

		if (dy01 == 0) {
			// the bottom part is flat horizontally
			drawHorizontalLine(this->p1.y, this->p0.x, this->p1.x, color);
		} else {
			int dx01 = this->p1.x - this->p0.x;
			double m01 = dx01 / dy01;
			double x01 = this->p0.x;

			for (int y = this->p0.y; y < this->p1.y; y++) {
				drawHorizontalLine(y, (int)x01, (int)x02, color);

				x01 += m01;
				x02 += m02;
			}
		}

		double dy12 = this->p2.y - this->p1.y;

		if (dy12 == 0) {
			// the top part is flat horizontally
			drawHorizontalLine(this->p2.y, this->p1.x, this->p2.x, color);
		} else {
			int dx12 = this->p2.x - this->p1.x;
			double m12 = dx12 / dy12;
			double x12 = this->p1.x;

			for (int y = this->p1.y; y <= this->p2.y; y++) {
				drawHorizontalLine(y, (int)x12, (int)x02, color);

				x12 += m12;
				x02 += m02;
			}
		}
	}

	void fillShaded() {
		//
	}
};

typedef struct {
	Point p0;
	Point p1;
	Point p2;
	int color;
} tri2d;

#endif
