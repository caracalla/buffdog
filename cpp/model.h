#ifndef BUFFDOG_MODEL
#define BUFFDOG_MODEL

#include <array>
#include <cmath>
#include <vector>

#include "device.h"
#include "triangle.h"
#include "vector.h"

// Logic for drawing models (currently, just cubes)

#define CUBE_V_COUNT 8
#define CUBE_T_COUNT 12

// 3D triangles are represented as indices in the model's vertex list
typedef struct {
	size_t v0;
	size_t v1;
	size_t v2;
	int color;
	Vector normal;
} tri3d;

Vector triangleNormal(Vector v0, Vector v1, Vector v2) {
	Vector side1 = v1.subtract(v0);
	Vector side2 = v2.subtract(v0);

	return side1.crossProduct(side2).unit();
}

struct Model {
	std::vector<Vector> vertices;
	std::vector<tri3d> triangles;
	double scale;
	Vector translation;
	Vector rotation; // represented as radians around each axis

	void setTriangleNormals() {
		for (auto& triangle : this->triangles) {
			triangle.normal = triangleNormal(
					this->vertices[triangle.v0],
					this->vertices[triangle.v1],
					this->vertices[triangle.v2]);
		}
	}
};

Model buildCube(double scale, Vector translation, Vector rotation) {
	double maxval = 0.8;
	double minval = 0.2;
	int red = device::color(maxval, minval, minval);
	int blue = device::color(minval, minval, maxval);
	int green = device::color(minval, maxval, minval);
	int yellow = device::color(maxval, maxval, minval);
	int purple = device::color(maxval, minval, maxval);
	int cyan = device::color(minval, maxval, maxval);

	Model item;

	item.vertices = {
			Vector::point( 1,  1,  1),
			Vector::point(-1,  1,  1),
			Vector::point(-1, -1,  1),
			Vector::point( 1, -1,  1),
			Vector::point( 1,  1, -1),
			Vector::point(-1,  1, -1),
			Vector::point(-1, -1, -1),
			Vector::point( 1, -1, -1)};

	item.triangles = {
			(tri3d){0, 1, 2, red},
			(tri3d){0, 2, 3, red},
			(tri3d){4, 0, 3, green},
			(tri3d){4, 3, 7, green},
			(tri3d){5, 4, 7, blue},
			(tri3d){5, 7, 6, blue},
			(tri3d){1, 5, 6, yellow},
			(tri3d){1, 6, 2, yellow},
			(tri3d){4, 5, 1, purple},
			(tri3d){4, 1, 0, purple},
			(tri3d){2, 6, 7, cyan},
			(tri3d){2, 7, 3, cyan}};

	item.scale = scale;
	item.translation = translation;
	item.rotation = rotation;

	item.setTriangleNormals();

	return item;
}

#endif
