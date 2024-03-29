#ifndef BUFFDOG_TRIANGLE
#define BUFFDOG_TRIANGLE

#include "../device.h"
#include "../line.h"
#include "../point.h"
#include "../vector.h"

#include "texture.h"


// Logic for drawing 2D triangles


int colorFromVector(Vector vec);

// mostly for debugging
void drawPoint(Point point, int color);



Vector getBarycentricWeights(Point p0, Point p1, Point p2, int x, int y);


// stupid naming conventions:
//   h represents lighting intensity
//   a is the step size between lighting intensities
//   q is a step size, generally for inverted z
//   qu and qv are texture coordinate step sizes
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
	int translucency = 0;

	// draws a wireframe triangle using its color value
	void draw();

	// draws a filled triangle using its color value
	void fill();

	// draws a triangle using its texture and lighting intensities using a
	// horizontal scanline approach
	void fillShaded();

	// draws a triangle using its texture and lighting intensities using a
	// barycentric bounding box approach
	void fillBarycentric();

	void drawShadedLine(
			int y,
			int x1,
			int x2,
			double h1,
			double h2,
			double inv_z1,
			double inv_z2,
			double inv_u1,
			double inv_v1,
			double inv_u2,
			double inv_v2);
};

#endif
