#ifndef BUFFDOG_MODEL
#define BUFFDOG_MODEL

#include <array>
#include <cmath>
#include <vector>

#include "vec4.h"
#include "triangle.h"

// Logic for drawing models (currently, just cubes)

#define CUBE_V_COUNT 8
#define CUBE_T_COUNT 12

// 3D triangles are represented as indices in the model's vertex list
typedef struct {
	size_t v0;
	size_t v1;
	size_t v2;
	int color;
} tri3d;

typedef struct {
	std::array<vec4, CUBE_V_COUNT> vertices;
	std::array<tri3d, CUBE_T_COUNT> triangles;
	double scale;
	vec4 translation;
	vec4 rotation; // represented as radians around each axis
} cube;

cube buildCube(double scale, vec4 translation, vec4 rotation) {
	int red = color(1, 0, 0);
	int blue = color(0, 0, 1);
	int green = color(0, 1, 0);
	int yellow = color(0, 0, 0); // color(1, 1, 0);
	int purple = color(1, 0, 1);
	int cyan = color(0, 1, 1);
	cube item = {
		{ // vertices
			vec4::point( 1,  1,  1),
			vec4::point(-1,  1,  1),
			vec4::point(-1, -1,  1),
			vec4::point( 1, -1,  1),
			vec4::point( 1,  1, -1),
			vec4::point(-1,  1, -1),
			vec4::point(-1, -1, -1),
			vec4::point( 1, -1, -1),
		},
		{ // triangles
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
			(tri3d){2, 7, 3, cyan},
		},
		scale,
		translation,
		rotation
	};

	return item;
}

#endif
