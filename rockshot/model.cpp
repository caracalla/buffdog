#include "../util.h"

#include "model.h"


Vector triangleNormal(Vector v0, Vector v1, Vector v2) {
	Vector side1 = v1.subtract(v0);
	Vector side2 = v2.subtract(v0);

	return side1.crossProduct(side2).unit();
}

Vector Triangle3D::getNormal() {
	// this doesn't work, because this->vN are indices within the model, not
	// actual Vectors
	// return triangleNormal(this->v0, this->v1, this->v2);

	return this->normal;
}

void Model::setTriangleNormals() {
	for (auto& triangle : this->triangles) {
		triangle.normal = triangleNormal(
				this->vertices[triangle.v0.index],
				this->vertices[triangle.v1.index],
				this->vertices[triangle.v2.index]);
	}
}

#define MAX_COLOR_VAL 0.9
#define MIN_COLOR_VAL 0.0

Model buildCube() {
	Vector start = Vector::point(-1, -1, -1);
	Vector end = Vector::point(1, 1, 1);

	return buildHexahedron(start, end);
}

Model buildHexahedron(Vector box_min, Vector box_max) {
	Vector red = Vector::color(MAX_COLOR_VAL, MIN_COLOR_VAL, MIN_COLOR_VAL);
	Vector blue = Vector::color(MIN_COLOR_VAL, MIN_COLOR_VAL, MAX_COLOR_VAL);
	Vector green = Vector::color(MIN_COLOR_VAL, MAX_COLOR_VAL, MIN_COLOR_VAL);
	Vector yellow = Vector::color(MAX_COLOR_VAL, MAX_COLOR_VAL, MIN_COLOR_VAL);
	Vector purple = Vector::color(MAX_COLOR_VAL, MIN_COLOR_VAL, MAX_COLOR_VAL);
	Vector cyan = Vector::color(MIN_COLOR_VAL, MAX_COLOR_VAL, MAX_COLOR_VAL);

	Model item;

	item.vertices = {
			Vector::point(box_max.x, box_max.y, box_max.z),   // 0
			Vector::point(box_min.x, box_max.y, box_max.z),   // 1
			Vector::point(box_min.x, box_min.y, box_max.z),   // 2
			Vector::point(box_max.x, box_min.y, box_max.z),   // 3
			Vector::point(box_max.x, box_max.y, box_min.z),   // 4
			Vector::point(box_min.x, box_max.y, box_min.z),   // 5
			Vector::point(box_min.x, box_min.y, box_min.z),   // 6
			Vector::point(box_max.x, box_min.y, box_min.z)};  // 7

	item.normals = {
			triangleNormal(item.vertices[0], item.vertices[1], item.vertices[3]),
			triangleNormal(item.vertices[4], item.vertices[0], item.vertices[3]),
			triangleNormal(item.vertices[5], item.vertices[4], item.vertices[7]),
			triangleNormal(item.vertices[1], item.vertices[5], item.vertices[6]),
			triangleNormal(item.vertices[4], item.vertices[5], item.vertices[1]),
			triangleNormal(item.vertices[2], item.vertices[6], item.vertices[7])};

	item.uvs = {
			std::make_pair(0.0, 0.0),
			std::make_pair(1.0, 0.0),
			std::make_pair(0.0, 1.0),
			std::make_pair(1.0, 1.0)};

	item.triangles = {
			// +z
			Triangle3D{
					Vertex{0, 0, 0, 1.0},
					Vertex{1, 0, 2, 1.0},
					Vertex{2, 0, 3, 1.0},
					blue},
			Triangle3D{
					Vertex{0, 0, 0, 1.0},
					Vertex{2, 0, 3, 1.0},
					Vertex{3, 0, 1, 1.0},
					blue},
			// +x
			Triangle3D{
					Vertex{4, 1, 0, 1.0},
					Vertex{0, 1, 2, 1.0},
					Vertex{3, 1, 3, 1.0},
					red},
			Triangle3D{
					Vertex{4, 1, 0, 1.0},
					Vertex{3, 1, 3, 1.0},
					Vertex{7, 1, 1, 1.0},
					red},
			// -x
			Triangle3D{
					Vertex{5, 2, 0, 1.0},
					Vertex{4, 2, 2, 1.0},
					Vertex{7, 2, 3, 1.0},
					cyan},
			Triangle3D{
					Vertex{5, 2, 0, 1.0},
					Vertex{7, 2, 3, 1.0},
					Vertex{6, 2, 1, 1.0},
					cyan},
			// -z
			Triangle3D{
					Vertex{1, 3, 0, 1.0},
					Vertex{5, 3, 2, 1.0},
					Vertex{6, 3, 3, 1.0},
					yellow},
			Triangle3D{
					Vertex{1, 3, 0, 1.0},
					Vertex{6, 3, 3, 1.0},
					Vertex{2, 3, 1, 1.0},
					yellow},
			// +y
			Triangle3D{
					Vertex{4, 4, 0, 1.0},
					Vertex{5, 4, 2, 1.0},
					Vertex{1, 4, 3, 1.0},
					green},
			Triangle3D{
					Vertex{4, 4, 0, 1.0},
					Vertex{1, 4, 3, 1.0},
					Vertex{0, 4, 1, 1.0},
					green},
			// -y
			Triangle3D{
					Vertex{2, 5, 0, 1.0},
					Vertex{6, 5, 2, 1.0},
					Vertex{7, 5, 3, 1.0},
					purple},
			Triangle3D{
					Vertex{2, 5, 0, 1.0},
					Vertex{7, 5, 3, 1.0},
					Vertex{3, 5, 1, 1.0},
					purple},
	};

	item.setTriangleNormals();

	return item;
}

Model buildTetrahedron() {
	Vector red = Vector::color(MAX_COLOR_VAL, MIN_COLOR_VAL, MIN_COLOR_VAL);
	Vector blue = Vector::color(MIN_COLOR_VAL, MIN_COLOR_VAL, MAX_COLOR_VAL);
	Vector green = Vector::color(MIN_COLOR_VAL, MAX_COLOR_VAL, MIN_COLOR_VAL);
	Vector purple = Vector::color(MAX_COLOR_VAL, MIN_COLOR_VAL, MAX_COLOR_VAL);

	Model item;

	double squirt3 = sqrt(3);
	double height = squirt3;
	double base_width = 0.5;

	item.vertices = {
			Vector::point( 0,          0,       0),
			Vector::point( 0,          -height,  2 * base_width / squirt3),
			Vector::point( base_width, -height, -base_width / squirt3),
			Vector::point(-base_width, -height, -base_width / squirt3)};

	item.normals = {
			triangleNormal(item.vertices[0], item.vertices[1], item.vertices[2]),
			triangleNormal(item.vertices[0], item.vertices[3], item.vertices[1]),
			triangleNormal(item.vertices[0], item.vertices[2], item.vertices[3]),
			triangleNormal(item.vertices[1], item.vertices[3], item.vertices[2])};

	item.uvs = {
			std::make_pair(0.0, 0.0),
			std::make_pair(1.0, 0.0),
			std::make_pair(0.5, 1.0)};

	item.triangles = {
			Triangle3D{
					Vertex{0, 0, 0, 1.0},
					Vertex{1, 0, 1, 1.0},
					Vertex{2, 0, 2, 1.0},
					red},
			Triangle3D{
					Vertex{0, 1, 0, 1.0},
					Vertex{3, 1, 1, 1.0},
					Vertex{1, 1, 2, 1.0},
					green},
			Triangle3D{
					Vertex{0, 2, 0, 1.0},
					Vertex{2, 2, 1, 1.0},
					Vertex{3, 2, 2, 1.0},
					blue},
			Triangle3D{
					Vertex{1, 3, 0, 1.0},
					Vertex{3, 3, 1, 1.0},
					Vertex{2, 3, 2, 1.0},
					purple}};

	item.setTriangleNormals();

	// keep it pointing up
	item.initial_rotation = Vector::direction(0, 0, 0);

	return item;
}

// generate a random color between red and yellow
Vector randomExplosionColor() {
	double max_color = util::randomDouble(0.5, 1.0);
	return Vector::color(max_color, util::randomDouble(0.0, max_color), 0.0);
}

Model buildIcosahedron() {
	Model item;

	// distance between vertex to center of icosahedron
	double radius = 1;

	// distance between two vertices
	double edge_length = radius / sin(2 * kPi / 5);

	// the distance from the center of the pentagon to a vertex
	double pentagon_spoke_length = edge_length / (2 * sin(kPi / 5));

	// angle between pentagon and a line between v0 t0 v1
	double alpha = acos(sqrt(0.5 + sqrt(5) / 10));

	// distance from center of icosahedron to the pentagon containing v1 to v5
	double pentagon_y = radius - (edge_length * sin(alpha));

	double v1_z = pentagon_spoke_length;

	double v2_x = edge_length * cos(kPi / 5);
	double v2_z = v1_z - edge_length * sin(kPi / 5);

	double v3_x = edge_length / 2;
	double v3_z = -edge_length * cos(kPi / 5) / (2 * sin(kPi / 5));

	item.vertices = {
			// first vertex is top
			Vector::point(0, radius, 0),

			// next five vertices are the upper pentagon, clockwise about the y axis
			Vector::point( 0,     pentagon_y, v1_z),
			Vector::point( v2_x,  pentagon_y, v2_z),
			Vector::point( v3_x,  pentagon_y, v3_z),
			Vector::point(-v3_x,  pentagon_y, v3_z),
			Vector::point(-v2_x,  pentagon_y, v2_z),

			// next five vertices are the lower pentagon, clockwise about the y axis
			Vector::point( 0,     -pentagon_y, -v1_z),
			Vector::point(-v2_x,  -pentagon_y, -v2_z),
			Vector::point(-v3_x,  -pentagon_y, -v3_z),
			Vector::point( v3_x,  -pentagon_y, -v3_z),
			Vector::point( v2_x,  -pentagon_y, -v2_z),

			// final vertex is the bottom
			Vector::point(0, -radius, 0)
	};

	// ignore these for now
	// item.normals = {
	// 		triangleNormal(item.vertices[0], item.vertices[1], item.vertices[2]),
	// 		triangleNormal(item.vertices[0], item.vertices[3], item.vertices[1]),
	// 		triangleNormal(item.vertices[0], item.vertices[2], item.vertices[3]),
	// 		triangleNormal(item.vertices[1], item.vertices[3], item.vertices[2])};

	// fake uv for now
	item.uvs = {
			std::make_pair(0.0, 0.0)
	};

	item.triangles = {
			// top pentagonal pyramid
			Triangle3D{
					Vertex{0, 0, 0, 1.0},
					Vertex{1, 0, 0, 1.0},
					Vertex{2, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{0, 0, 0, 1.0},
					Vertex{2, 0, 0, 1.0},
					Vertex{3, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{0, 0, 0, 1.0},
					Vertex{3, 0, 0, 1.0},
					Vertex{4, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{0, 0, 0, 1.0},
					Vertex{4, 0, 0, 1.0},
					Vertex{5, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{0, 0, 0, 1.0},
					Vertex{5, 0, 0, 1.0},
					Vertex{1, 0, 0, 1.0},
					randomExplosionColor()},

			// central pentagonal antiprism
			Triangle3D{
					Vertex{1, 0, 0, 1.0},
					Vertex{9, 0, 0, 1.0},
					Vertex{2, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{9, 0, 0, 1.0},
					Vertex{10, 0, 0, 1.0},
					Vertex{2, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{2, 0, 0, 1.0},
					Vertex{10, 0, 0, 1.0},
					Vertex{3, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{10, 0, 0, 1.0},
					Vertex{6, 0, 0, 1.0},
					Vertex{3, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{3, 0, 0, 1.0},
					Vertex{6, 0, 0, 1.0},
					Vertex{4, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{6, 0, 0, 1.0},
					Vertex{7, 0, 0, 1.0},
					Vertex{4, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{4, 0, 0, 1.0},
					Vertex{7, 0, 0, 1.0},
					Vertex{5, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{7, 0, 0, 1.0},
					Vertex{8, 0, 0, 1.0},
					Vertex{5, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{5, 0, 0, 1.0},
					Vertex{8, 0, 0, 1.0},
					Vertex{1, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{8, 0, 0, 1.0},
					Vertex{9, 0, 0, 1.0},
					Vertex{1, 0, 0, 1.0},
					randomExplosionColor()},

			// bottom pentagonal pyramid
			Triangle3D{
					Vertex{11, 0, 0, 1.0},
					Vertex{7, 0, 0, 1.0},
					Vertex{6, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{11, 0, 0, 1.0},
					Vertex{8, 0, 0, 1.0},
					Vertex{7, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{11, 0, 0, 1.0},
					Vertex{9, 0, 0, 1.0},
					Vertex{8, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{11, 0, 0, 1.0},
					Vertex{10, 0, 0, 1.0},
					Vertex{9, 0, 0, 1.0},
					randomExplosionColor()},
			Triangle3D{
					Vertex{11, 0, 0, 1.0},
					Vertex{6, 0, 0, 1.0},
					Vertex{10, 0, 0, 1.0},
					randomExplosionColor()}
		};

	item.setTriangleNormals();

	item.compute_lighting = false;

	return item;
}

Vector midpoint(Vector& v1, Vector& v2) {
	return Vector::point(
		(v1.x + v2.x) / 2,
		(v1.y + v2.y) / 2,
		(v1.z + v2.z) / 2);
}

Model subdivide(Model model) {
	// for each triangle
	//   add vertex v01 between v0 and v1
	//   add vertex v12 between v1 and v2
	//   add vertex v20 between v2 and v0
	//   make triangle v0  v01 v20
	//   make triangle v01 v1  v12
	//   make triangle v20 v12 v2
	//   make triangle v01 v12 v20

	std::vector<Triangle3D> new_triangles;
	// new_triangles.reserve(model.triangles.size() * 4);

	for (auto& triangle : model.triangles) {
		size_t v0_index = triangle.v0.index;
		size_t v1_index = triangle.v1.index;
		size_t v2_index = triangle.v2.index;

		Vector v0 = model.vertices[v0_index];
		Vector v1 = model.vertices[v1_index];
		Vector v2 = model.vertices[v2_index];

		// this will create duplicate vertices.  Ideally, I would avoid this by
		// keeping track of already created ones, but that would require sorting
		// vertices somehow, making a map, etc
		Vector v01 = midpoint(v0, v1).unit();
		size_t v01_index = model.vertices.size();
		model.vertices.push_back(v01);
		Vector v12 = midpoint(v1, v2).unit();
		size_t v12_index = model.vertices.size();
		model.vertices.push_back(v12);
		Vector v20 = midpoint(v2, v0).unit();
		size_t v20_index = model.vertices.size();
		model.vertices.push_back(v20);

		new_triangles.push_back(
				Triangle3D{
						Vertex{v0_index,  0, 0, 1.0},
						Vertex{v01_index, 0, 0, 1.0},
						Vertex{v20_index, 0, 0, 1.0},
						randomExplosionColor()});

		new_triangles.push_back(
				Triangle3D{
						Vertex{v01_index, 0, 0, 1.0},
						Vertex{v1_index,  0, 0, 1.0},
						Vertex{v12_index, 0, 0, 1.0},
						randomExplosionColor()});

		new_triangles.push_back(
				Triangle3D{
						Vertex{v20_index, 0, 0, 1.0},
						Vertex{v12_index, 0, 0, 1.0},
						Vertex{v2_index,  0, 0, 1.0},
						randomExplosionColor()});

		new_triangles.push_back(
				Triangle3D{
						Vertex{v01_index, 0, 0, 1.0},
						Vertex{v12_index, 0, 0, 1.0},
						Vertex{v20_index, 0, 0, 1.0},
						randomExplosionColor()});
	}

	model.triangles = std::move(new_triangles);

	model.setTriangleNormals();

	return model;
}
