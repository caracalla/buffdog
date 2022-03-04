#ifndef BUFFDOG_MODEL
#define BUFFDOG_MODEL

#include <vector>
#include <utility>

#include "../vector.h"

#include "texture.h"
#include "triangle.h"


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
	bool ignore_texture = false; // draw as a solid color

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

	Vector getNormal();
};

struct Model {
	std::vector<Vector> vertices;
	std::vector<Vector> normals; // these are technically optional
	std::vector<std::pair<double, double> > uvs;

	std::vector<Triangle3D> triangles;

	Texture* texture;
	bool has_texture = false;

	// hack for explosions
	bool compute_lighting = true;

	// this should be a vector indicating how the model should be rotated
	// initially to in a "neutral" position
	// what constitutes "neutral" is open to interpretation; i.e. is a rocket
	// "neutral" when it's lying on its side, or when the tip is pointing up?
	Vector initial_rotation;

	int translucency = 0;

	// TODO: does precomputing triangle normals make sense?
	// maybe not, but it's hard to do otherwise sadly
	void setTriangleNormals();

	void setTexture(Texture* texture) {
		this->texture = texture;
		this->has_texture = true;
	}

	// this is kind of useless for logging regular models, but it can be useful
	// for world models
	void log(const char* identifier = "model") {
		Vector bbox_max;
		Vector bbox_min;

		for (auto& vertex : this->vertices) {
			for (int i = 0; i < 3; i++) {
				if (vertex.at(i) > bbox_max.at(i)) {
					bbox_max.at(i) = vertex.at(i);
				}

				if (vertex.at(i) < bbox_min.at(i)) {
					bbox_min.at(i) = vertex.at(i);
				}
			}
		}

		device::logOncePerSecond("model %s: \n", identifier);
		bbox_max.log("    max");
		bbox_min.log("    min");
	}

	// convenience functions for basic primitives
	static Model buildHexahedron(Vector box_min, Vector box_max); // technically, this creates a rectangular cuboid (don't tell anyone)
	static Model buildCube();
	static Model buildTetrahedron();
	static Model buildIcosahedron();
	static Model buildExplosionModel();
};


#endif
