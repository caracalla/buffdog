#ifndef BUFFDOG_MODEL
#define BUFFDOG_MODEL

#include <math.h>

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
	vec4 vertices[8];
	tri3d triangles[12];
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
			{0, 1, 2, red},
			{0, 2, 3, red},
			{4, 0, 3, green},
			{4, 3, 7, green},
			{5, 4, 7, blue},
			{5, 7, 6, blue},
			{1, 5, 6, yellow},
			{1, 6, 2, yellow},
			{4, 5, 1, purple},
			{4, 1, 0, purple},
			{2, 6, 7, cyan},
			{2, 7, 3, cyan},
		},
		scale,
		translation,
		rotation
	};

	return item;
}

#endif
