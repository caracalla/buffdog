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
		double u1,
		double v1,
		double u2,
		double v2,
		Vector color,
		bool has_texture,
		Texture* texture) {
	int start_x;
	int end_x;

	double h;
	double a;

	double z;
	double q;

	double u;
	double v;

	double uq;
	double vq;

	if (x1 < x2) {
		start_x = (int)x1;
		end_x = (int)x2;

		double dx = (x2 - x1);

		h = h1;
		a = (h2 - h1) / dx;

		z = z1;
		q = (z2 - z1) / dx;

		u = u1;
		uq = (u2 - u1) / dx;

		v = v1;
		vq = (v2 - v1) / dx;
	} else {
		start_x = (int)x2;
		end_x = (int)x1;

		double dx = (x1 - x2);

		h = h2;
		a = (h1 - h2) / dx;

		z = z2;
		q = (z1 - z2) / dx;

		u = u2;
		uq = (u1 - u2) / dx;

		v = v2;
		vq = (v1 - v2) / dx;
	}

	for (int x = start_x; x < end_x; x++) {
		double& z_buffer_value = device::zBufferAt(x, y);

		if (z < z_buffer_value) {
			uint32_t final_color;

			if (has_texture) {
				Vector vec_color = texture->vectorColorFromUV(u / z, v / z);
				final_color = colorFromVector(vec_color.scalarMultiply(h));
			} else {
				final_color = colorFromVector(color.scalarMultiply(h));
			}
			device::setPixel(x, y, final_color);

			z_buffer_value = z;
		}

		h += a;
		z += q;

		u += uq;
		v += vq;
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
	double u0;
	double v0;
	double u1;
	double v1;
	double u2;
	double v2;
	bool has_texture = false;
	Texture* texture;

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
		double utemp;
		double vtemp;

		double inv_u0 = this->u0 * this->invZ0;
		double inv_v0 = this->v0 * this->invZ0;

		double inv_u1 = this->u1 * this->invZ1;
		double inv_v1 = this->v1 * this->invZ1;

		double inv_u2 = this->u2 * this->invZ2;
		double inv_v2 = this->v2 * this->invZ2;

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

			utemp = inv_u0;
			inv_u0 = inv_u1;
			inv_u1 = utemp;

			vtemp = inv_v0;
			inv_v0 = inv_v1;
			inv_v1 = vtemp;
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

			utemp = inv_u0;
			inv_u0 = inv_u2;
			inv_u2 = utemp;

			vtemp = inv_v0;
			inv_v0 = inv_v2;
			inv_v2 = vtemp;
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

			utemp = inv_u1;
			inv_u1 = inv_u2;
			inv_u2 = utemp;

			vtemp = inv_v1;
			inv_v1 = inv_v2;
			inv_v2 = vtemp;
		}

		double dy01 = this->p1.y - this->p0.y;
		double dy12 = this->p2.y - this->p1.y;
		double dy02 = this->p2.y - this->p0.y;

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
					inv_u0,
					inv_u1,
					inv_v0,
					inv_v1,
					this->color,
					this->has_texture,
					this->texture);

			drawShadedLine(
					this->p2.y,
					this->p1.x,
					this->p2.x,
					this->h1,
					this->h2,
					this->invZ1,
					this->invZ2,
					inv_u1,
					inv_u2,
					inv_v1,
					inv_v2,
					this->color,
					this->has_texture,
					this->texture);
			return;
		}

		double x01 = this->p0.x;
		double x12 = this->p1.x;
		double x02 = this->p0.x;

		double h01 = this->h0;
		double h12 = this->h1;
		double h02 = this->h0;

		double z01 = this->invZ0;
		double z12 = this->invZ1;
		double z02 = this->invZ0;

		double u01 = inv_u0;
		double u12 = inv_u1;
		double u02 = inv_u0;

		double v01 = inv_v0;
		double v12 = inv_v1;
		double v02 = inv_v0;

		double m02 = (this->p2.x - this->p0.x) / dy02;
		double a02 = (this->h2 - this->h0) / dy02;
		double q02 = (this->invZ2 - this->invZ0) / dy02;
		double uq02 = (inv_u2 - inv_u0) / dy02;
		double vq02 = (inv_v2 - inv_v0) / dy02;

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
					inv_u0,
					inv_v0,
					inv_u1,
					inv_v1,
					this->color,
					this->has_texture,
					this->texture);

		} else {
			double m01 = (this->p1.x - this->p0.x) / dy01;
			double a01 = (this->h1 - this->h0) / dy01;
			double q01 = (this->invZ1 - this->invZ0) / dy01;
			double uq01 = (inv_u1 - inv_u0) / dy01;
			double vq01 = (inv_v1 - inv_v0) / dy01;

			for (int y = this->p0.y; y < this->p1.y; y++) {
				drawShadedLine(y, x01, x02, h01, h02, z01, z02, u01, v01, u02, v02, this->color, this->has_texture, this->texture);

				x01 += m01;
				x02 += m02;

				h01 += a01;
				h02 += a02;

				z01 += q01;
				z02 += q02;

				u01 += uq01;
				u02 += uq02;

				v01 += vq01;
				v02 += vq02;

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
					inv_u1,
					inv_v1,
					inv_u2,
					inv_v2,
					this->color,
					this->has_texture,
					this->texture);
		} else {
			double m12 = (this->p2.x - this->p1.x) / dy12;
			double a12 = (this->h2 - this->h1) / dy12;
			double q12 = (this->invZ2 - this->invZ1) / dy12;
			double uq12 = (inv_u2 - inv_u1) / dy12;
			double vq12 = (inv_v2 - inv_v1) / dy12;

			for (int y = this->p1.y; y <= this->p2.y; y++) {
				drawShadedLine(y, x12, x02, h12, h02, z12, z02, u12, v12, u02, v02, this->color, this->has_texture, this->texture);

				x12 += m12;
				x02 += m02;

				h12 += a12;
				h02 += a02;

				z12 += q12;
				z02 += q02;

				u12 += uq12;
				u02 += uq02;

				v12 += vq12;
				v02 += vq02;
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
