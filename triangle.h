#ifndef BUFFDOG_TRIANGLE
#define BUFFDOG_TRIANGLE

#include <cmath>

#include "device.h"
#include "line.h"
#include "vector.h"


// #define min(a, b) (a < b ? a : b)
// #define max(a, b) (a > b ? a : b)

int min(int a, int b) { return (a < b ? a : b); }
int max(int a, int b) { return (a > b ? a : b); }


// Logic for drawing 2D triangles

// stupid naming conventions:
//   h represents lighting intensity
//   a is the step size between lighting intensities
//   q is a step size, generally for inverted z
//   qu and qv are texture coordinate step sizes

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
		double inv_z1,
		double inv_z2,
		double inv_u1,
		double inv_v1,
		double inv_u2,
		double inv_v2,
		Vector color,
		Texture* texture) {
	int start_x;
	int end_x;

	double h;
	double a;

	double inv_z;
	double q;

	double inv_u;
	double inv_v;

	double inv_uq;
	double inv_vq;

	if (x1 < x2) {
		start_x = (int)x1;
		end_x = (int)x2;

		double dx = (x2 - x1);

		h = h1;
		a = (h2 - h1) / dx;

		inv_z = inv_z1;
		q = (inv_z2 - inv_z1) / dx;

		inv_u = inv_u1;
		inv_uq = (inv_u2 - inv_u1) / dx;

		inv_v = inv_v1;
		inv_vq = (inv_v2 - inv_v1) / dx;
	} else {
		start_x = (int)x2;
		end_x = (int)x1;

		double dx = (x1 - x2);

		h = h2;
		a = (h1 - h2) / dx;

		inv_z = inv_z2;
		q = (inv_z1 - inv_z2) / dx;

		inv_u = inv_u2;
		inv_uq = (inv_u1 - inv_u2) / dx;

		inv_v = inv_v2;
		inv_vq = (inv_v1 - inv_v2) / dx;
	}

	for (int x = start_x; x < end_x; x++) {
		double& z_buffer_value = device::zBufferAt(x, y);

		if (inv_z < z_buffer_value) {
			uint32_t final_color;

			if (texture) {
				Vector vec_color = texture->vectorColorFromUV(inv_u / inv_z, inv_v / inv_z);
				final_color = colorFromVector(vec_color.scalarMultiply(h));
			} else {
				final_color = colorFromVector(color.scalarMultiply(h));
			}
			device::setPixel(x, y, final_color);

			z_buffer_value = inv_z;
		}

		h += a;
		inv_z += q;

		inv_u += inv_uq;
		inv_v += inv_vq;
	}
}

Vector getBarycentricWeights(Point p0, Point p1, Point p2, int x, int y) {
	Vector px = {(double)p2.x - p0.x, (double)p1.x - p0.x, (double)p0.x - x};
	Vector py = {(double)p2.y - p0.y, (double)p1.y - p0.y, (double)p0.y - y};

	Vector u = px.crossProduct(py);

	if (fabs(u.z) < 1) {
		return Vector{-1, 1, 1};
	}

	return Vector{1.0 - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z};
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
	Texture* texture;

	void draw() {
		int color = colorFromVector(this->color);
		drawLineFromPoints(this->p0, this->p1, color);
		drawLineFromPoints(this->p1, this->p2, color);
		drawLineFromPoints(this->p2, this->p0, color);
	}

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
					inv_v1,
					inv_u2,
					inv_v2,
					this->color,
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
					this->texture);

		} else {
			double m01 = (this->p1.x - this->p0.x) / dy01;
			double a01 = (this->h1 - this->h0) / dy01;
			double q01 = (this->invZ1 - this->invZ0) / dy01;
			double uq01 = (inv_u1 - inv_u0) / dy01;
			double vq01 = (inv_v1 - inv_v0) / dy01;

			for (int y = this->p0.y; y < this->p1.y; y++) {
				drawShadedLine(y, x01, x02, h01, h02, z01, z02, u01, v01, u02, v02, this->color, this->texture);

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
					this->texture);
		} else {
			double m12 = (this->p2.x - this->p1.x) / dy12;
			double a12 = (this->h2 - this->h1) / dy12;
			double q12 = (this->invZ2 - this->invZ1) / dy12;
			double uq12 = (inv_u2 - inv_u1) / dy12;
			double vq12 = (inv_v2 - inv_v1) / dy12;

			for (int y = this->p1.y; y <= this->p2.y; y++) {
				drawShadedLine(y, x12, x02, h12, h02, z12, z02, u12, v12, u02, v02, this->color, this->texture);

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

	void fillBarycentric() {
		double inv_u0 = this->u0 * this->invZ0;
		double inv_v0 = this->v0 * this->invZ0;

		double inv_u1 = this->u1 * this->invZ1;
		double inv_v1 = this->v1 * this->invZ1;

		double inv_u2 = this->u2 * this->invZ2;
		double inv_v2 = this->v2 * this->invZ2;

		// define the bounding box containing the triangle
		Point bmin = {
				min(this->p0.x, min(this->p1.x, this->p2.x)),
				min(this->p0.y, min(this->p1.y, this->p2.y))};
		Point bmax = {
				max(this->p0.x, max(this->p1.x, this->p2.x)),
				max(this->p0.y, max(this->p1.y, this->p2.y))};

		for (int x = bmin.x; x <= bmax.x; x++) {
			for (int y = bmin.y; y <= bmax.y; y++) {
				Vector bc_weights = getBarycentricWeights(this->p0, this->p1, this->p2, x, y);

				if (bc_weights.x < 0 || bc_weights.y < 0 || bc_weights.z < 0) {
					continue;
				}

				double h = this->h0 * bc_weights.x + this->h1 * bc_weights.y + this->h2 * bc_weights.z;
				double inv_z = this->invZ0 * bc_weights.x + this->invZ1 * bc_weights.y + this->invZ2 * bc_weights.z;
				double inv_u = inv_u0 * bc_weights.x + inv_u1 * bc_weights.y + inv_u2 * bc_weights.z;
				double inv_v = inv_v0 * bc_weights.x + inv_v1 * bc_weights.y + inv_v2 * bc_weights.z;

				double& z_buffer_value = device::zBufferAt(x, y);

				if (inv_z < z_buffer_value) {
					uint32_t final_color;

					if (this->texture) {
						Vector vec_color = texture->vectorColorFromUV(inv_u / inv_z, inv_v / inv_z);
						final_color = colorFromVector(vec_color.scalarMultiply(h));
					} else {
						final_color = colorFromVector(color.scalarMultiply(h));
					}

					device::setPixel(x, y, final_color);

					z_buffer_value = inv_z;
				}
			}
		}
	}
};

#endif
