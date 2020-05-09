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

void drawShadedLine(
		double y,
		double x1,
		double x2,
		double h1,
		double h2,
		double z1,
		double z2,
		Vector color) {
	int start_x;
	int end_x;

	double h;
	double a;

	double z;
	double q;

	if (x1 < x2) {
		start_x = (int)x1;
		end_x = (int)x2;

		h = h1;
		a = (h2 - h1) / (x2 - x1);

		z = z1;
		q = (z2 - z1) / (x2 - x1);
	} else {
		start_x = (int)x2;
		end_x = (int)x1;

		h = h2;
		a = (h1 - h2) / (x1 - x2);

		z = z2;
		q = (z1 - z2) / (x1 - x2);
	}

	for (int x = start_x; x < end_x; x++) {
		double& z_buffer_value = device::zBufferAt(x, y);

		if (z < z_buffer_value) {
			device::setPixel(x, y, colorFromVector(color.scalarMultiply(h)));

			z_buffer_value = z;
		}

		h += a;
		z += q;
	}
}

struct Triangle2D {
	Point p0;
	Point p1;
	Point p2;
	Vector color;
	double h0;
	double h1;
	double h2;
	double invZ0;
	double invZ1;
	double invZ2;

	void draw() {
		int color = colorFromVector(this->color);
		drawLineFromPoints(this->p0, this->p1, color);
		drawLineFromPoints(this->p1, this->p2, color);
		drawLineFromPoints(this->p2, this->p0, color);
	}

	// void fill() {
	// 	int color = colorFromVector(this->color);
	//
	// 	// sort from highest (p2) to lowest (p0)
	// 	Point temp;
	// 	if (this->p0.y > this->p1.y) {
	// 		temp = this->p0;
	// 		this->p0 = this->p1;
	// 		this->p1 = temp;
	// 	}
	//
	// 	if (this->p0.y > this->p2.y) {
	// 		temp = this->p0;
	// 		this->p0 = this->p2;
	// 		this->p2 = temp;
	// 	}
	//
	// 	if (this->p1.y > this->p2.y) {
	// 		temp = this->p2;
	// 		this->p2 = this->p1;
	// 		this->p1 = temp;
	// 	}
	//
	// 	// po to p2 is the long vertical side
	// 	double dy02 = this->p2.y - this->p0.y;
	//
	// 	if (dy02 == 0) {
	// 		// the whole thing is flat horizontally
	// 		// draw a horizontal line from the min x to the max x
	// 		int min_x = THREE_MIN(this->p0.x, this->p1.x, this->p2.x);
	// 		int max_x = THREE_MAX(this->p0.x, this->p1.x, this->p2.x);
	//
	// 		drawHorizontalLine(this->p2.y, min_x, max_x, color);
	// 		return;
	// 	}
	//
	// 	int dx02 = this->p2.x - this->p0.x;
	// 	double m02 = dx02 / dy02;
	// 	// the x value for line p2 - p0 changes constantly over the whole triangle
	// 	double x02 = this->p0.x;
	//
	// 	// p0 to p1 is the bottom half
	// 	double dy01 = this->p1.y - this->p0.y;
	//
	// 	if (dy01 == 0) {
	// 		// the bottom part is flat horizontally
	// 		drawHorizontalLine(this->p1.y, this->p0.x, this->p1.x, color);
	// 	} else {
	// 		int dx01 = this->p1.x - this->p0.x;
	// 		double m01 = dx01 / dy01;
	// 		double x01 = this->p0.x;
	//
	// 		for (int y = this->p0.y; y < this->p1.y; y++) {
	// 			drawHorizontalLine(y, (int)x01, (int)x02, color);
	//
	// 			x01 += m01;
	// 			x02 += m02;
	// 		}
	// 	}
	//
	// 	double dy12 = this->p2.y - this->p1.y;
	//
	// 	if (dy12 == 0) {
	// 		// the top part is flat horizontally
	// 		drawHorizontalLine(this->p2.y, this->p1.x, this->p2.x, color);
	// 	} else {
	// 		int dx12 = this->p2.x - this->p1.x;
	// 		double m12 = dx12 / dy12;
	// 		double x12 = this->p1.x;
	//
	// 		for (int y = this->p1.y; y <= this->p2.y; y++) {
	// 			drawHorizontalLine(y, (int)x12, (int)x02, color);
	//
	// 			x12 += m12;
	// 			x02 += m02;
	// 		}
	// 	}
	// }

	void fillShaded() {
		// sort from highest (p2) to lowest (p0)
		Point temp;
		double htemp;
		double invZtemp;

		if (this->p0.y > this->p1.y) {
			temp = this->p0;
			this->p0 = this->p1;
			this->p1 = temp;

			htemp = this->h0;
			this->h0 = this->h1;
			this->h1 = htemp;

			invZtemp = this->invZ0;
			this->invZ0 = this->invZ1;
			this->invZ1 = invZtemp;
		}

		if (this->p0.y > this->p2.y) {
			temp = this->p0;
			this->p0 = this->p2;
			this->p2 = temp;

			htemp = this->h0;
			this->h0 = this->h2;
			this->h2 = htemp;

			invZtemp = this->invZ0;
			this->invZ0 = this->invZ2;
			this->invZ2 = invZtemp;
		}

		if (this->p1.y > this->p2.y) {
			temp = this->p2;
			this->p2 = this->p1;
			this->p1 = temp;

			htemp = this->h1;
			this->h1 = this->h2;
			this->h2 = htemp;

			invZtemp = this->invZ1;
			this->invZ1 = this->invZ2;
			this->invZ2 = invZtemp;
		}

		double dy01 = this->p1.y - this->p0.y;
		double dy12 = this->p2.y - this->p1.y;
		double dy02 = this->p2.y - this->p0.y;

		double x01 = this->p0.x;
		double x12 = this->p1.x;
		double x02 = this->p0.x;

		double h01 = this->h0;
		double h12 = this->h1;
		double h02 = this->h0;

		double z01 = this->invZ0;
		double z12 = this->invZ1;
		double z02 = this->invZ2;

		if (dy02 == 0) {
			// the whole thing is flat horizontally
			// draw a horizontal line from the min x to the max x
			// int min_x = THREE_MIN(this->p0.x, this->p1.x, this->p2.x);
			// int max_x = THREE_MAX(this->p0.x, this->p1.x, this->p2.x);

			// this is kind of dumb
			drawShadedLine(
					this->p1.y,
					this->p0.x,
					this->p1.x,
					this->h0,
					this->h1,
					this->invZ0,
					this->invZ1,
					this->color);

			drawShadedLine(
					this->p2.y,
					this->p1.x,
					this->p2.x,
					this->h1,
					this->h2,
					this->invZ1,
					this->invZ2,
					this->color);
			return;
		}

		double m02 = (this->p2.x - this->p0.x) / dy02;
		double a02 = (this->h2 - this->h0) / dy02;
		double q02 = (this->invZ2 - this->invZ0) / dy02;

		if (dy01 == 0) {
			// the bottom part is flat horizontally
			drawShadedLine(
					this->p1.y,
					this->p0.x,
					this->p1.x,
					this->h0,
					this->h1,
					this->invZ0,
					this->invZ1,
					this->color);
		} else {
			double m01 = (this->p1.x - this->p0.x) / dy01;
			double a01 = (this->h1 - this->h0) / dy01;
			double q01 = (this->invZ1 - this->invZ0) / dy01;

			for (int y = this->p0.y; y < this->p1.y; y++) {
				drawShadedLine(y, x01, x02, h01, h02, z01, z02, this->color);

				x01 += m01;
				x02 += m02;

				h01 += a01;
				h02 += a02;

				z01 += q01;
				z02 += q02;
			}
		}

		if (dy12 == 0) {
			// the top part is flat horizontally
			drawShadedLine(
					this->p2.y,
					this->p1.x,
					this->p2.x,
					this->h1,
					this->h2,
					this->invZ1,
					this->invZ2,
					this->color);
		} else {
			double m12 = (this->p2.x - this->p1.x) / dy12;
			double a12 = (this->h2 - this->h1) / dy12;
			double q12 = (this->invZ2 - this->invZ1) / dy12;

			for (int y = this->p1.y; y <= this->p2.y; y++) {
				drawShadedLine(y, x12, x02, h12, h02, z12, z02, this->color);

				x12 += m12;
				x02 += m02;

				h12 += a12;
				h02 += a02;

				z12 += q12;
				z02 += q02;
			}
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
