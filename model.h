#ifndef BUFFDOG_MODEL
#define BUFFDOG_MODEL

#include <cmath>
#include <vector>
#include <utility>

#include "texture.h"
#include "device.h"
#include "triangle.h"
#include "vector.h"


// Logic for drawing models


struct Vertex {
	size_t index;
	size_t normal;
	size_t uv;
	double light_intensity;
};

struct Triangle3D {
	Vertex v0;
	Vertex v1;
	Vertex v2;
	Vector color;
	Vector normal;
	bool special = false;

	Vertex& at(size_t index) {
		switch(index) {
			case 0:
				return this->v0;
				break;

			case 1:
				return this->v1;
				break;

			case 2:
				return this->v2;
				break;

			default:
				terminateFatal("invalid Triangle3D Vertex index");
				return this->v0;
				break;
		}
	}
};

struct Model {
	std::vector<Vector> vertices;
	std::vector<Vector> normals;
	std::vector<std::pair<double, double> > uvs;

	std::vector<Triangle3D> triangles;

	Texture* texture;
	bool has_texture = false;

	void setTriangleNormals();

	void setTexture(Texture* texture) {
		this->texture = texture;
		this->has_texture = true;
	}
};

Model buildCube();
Model buildTetrahedron();

#endif
