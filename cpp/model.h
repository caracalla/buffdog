#ifndef BUFFDOG_MODEL
#define BUFFDOG_MODEL

#include <cmath>
#include <vector>

#include "bmp.h"
#include "triangle.h"
#include "vector.h"

// Logic for drawing models (currently, just cubes and tetrahedrons)

double MAX_COLOR_VAL = 0.9;
double MIN_COLOR_VAL = 0.0;


// 3D triangles are represented as indices in the model's vertex list
typedef struct {
	size_t v0;
	size_t v1;
	size_t v2;
	Vector color;
	double tex_u0;
	double tex_v0;
	double tex_u1;
	double tex_v1;
	double tex_u2;
	double tex_v2;
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
	std::vector<double> shades;
	double scale;
	Vector translation;
	Vector rotation; // represented as radians around each axis
	BMPTexture texture;

	void setTriangleNormals() {
		for (auto& triangle : this->triangles) {
			triangle.normal = triangleNormal(
					this->vertices[triangle.v0],
					this->vertices[triangle.v1],
					this->vertices[triangle.v2]);
		}
	}

	void addTexture(BMPTexture texture) {
		this->texture = texture;
	}
};

Model buildCube(double scale, Vector translation, Vector rotation) {
	Vector red = Vector::color(MAX_COLOR_VAL, MIN_COLOR_VAL, MIN_COLOR_VAL);
	Vector blue = Vector::color(MIN_COLOR_VAL, MIN_COLOR_VAL, MAX_COLOR_VAL);
	Vector green = Vector::color(MIN_COLOR_VAL, MAX_COLOR_VAL, MIN_COLOR_VAL);
	Vector yellow = Vector::color(MAX_COLOR_VAL, MAX_COLOR_VAL, MIN_COLOR_VAL);
	Vector purple = Vector::color(MAX_COLOR_VAL, MIN_COLOR_VAL, MAX_COLOR_VAL);
	Vector cyan = Vector::color(MIN_COLOR_VAL, MAX_COLOR_VAL, MAX_COLOR_VAL);

	Model item;

	item.vertices = {
			Vector::point( 1,  1,  1),   // 0
			Vector::point(-1,  1,  1),   // 1
			Vector::point(-1, -1,  1),   // 2
			Vector::point( 1, -1,  1),   // 3
			Vector::point( 1,  1, -1),   // 4
			Vector::point(-1,  1, -1),   // 5
			Vector::point(-1, -1, -1),   // 6
			Vector::point( 1, -1, -1)};  // 7

	item.triangles = {
			(tri3d){0, 1, 2, red, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0},
			(tri3d){0, 2, 3, red, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0},
			// (tri3d){0, 3, 2, red, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0},
			(tri3d){4, 0, 3, green, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0},
			(tri3d){4, 3, 7, green, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0},
			(tri3d){5, 4, 7, blue, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0},
			(tri3d){5, 7, 6, blue, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0},
			(tri3d){1, 5, 6, yellow, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0},
			(tri3d){1, 6, 2, yellow, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0},
			(tri3d){4, 5, 1, purple, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0},
			(tri3d){4, 1, 0, purple, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0},
			(tri3d){2, 6, 7, cyan, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0},
			(tri3d){2, 7, 3, cyan, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0}};

	item.shades = {
			1.0,
			0.0,
			1.0,
			0.0,
			0.0,
			1.0,
			0.0,
			1.0};

	item.scale = scale;
	item.translation = translation;
	item.rotation = rotation;

	item.setTriangleNormals();

	return item;
}

Model buildTetrahedron(double scale, Vector translation, Vector rotation) {
	Vector red = Vector::color(MAX_COLOR_VAL, MIN_COLOR_VAL, MIN_COLOR_VAL);
	Vector blue = Vector::color(MIN_COLOR_VAL, MIN_COLOR_VAL, MAX_COLOR_VAL);
	Vector green = Vector::color(MIN_COLOR_VAL, MAX_COLOR_VAL, MIN_COLOR_VAL);
	Vector black = Vector::color(MIN_COLOR_VAL, MIN_COLOR_VAL, MIN_COLOR_VAL);

	Model item;

	double squirt3 = sqrt(3);

	item.vertices = {
			Vector::point( 0, squirt3,  0),
			Vector::point( 0, 0,        2 / squirt3),
			Vector::point( 1, 0,       -1 / squirt3),
			Vector::point(-1, 0,       -1 / squirt3)};

	item.triangles = {
			(tri3d){0, 1, 2, red},
			(tri3d){0, 3, 1, green},
			(tri3d){0, 2, 3, blue},
			(tri3d){1, 3, 2, black}};

	item.shades = {1.0, 0.0, 0.0, 0.0};

	item.scale = scale;
	item.translation = translation;
	item.rotation = rotation;

	item.setTriangleNormals();

	return item;
}

#endif
