#include "model.h"


Vector triangleNormal(Vector v0, Vector v1, Vector v2) {
	Vector side1 = v1.subtract(v0);
	Vector side2 = v2.subtract(v0);

	return side1.crossProduct(side2).unit();
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
			Triangle3D{
					Vertex{0, 0, 0, 1.0},
					Vertex{1, 0, 2, 1.0},
					Vertex{2, 0, 3, 1.0},
					red},
			Triangle3D{
					Vertex{0, 0, 0, 1.0},
					Vertex{2, 0, 3, 1.0},
					Vertex{3, 0, 1, 1.0},
					red},
			Triangle3D{
					Vertex{4, 1, 0, 1.0},
					Vertex{0, 1, 2, 1.0},
					Vertex{3, 1, 3, 1.0},
					green},
			Triangle3D{
					Vertex{4, 1, 0, 1.0},
					Vertex{3, 1, 3, 1.0},
					Vertex{7, 1, 1, 1.0},
					green},
			Triangle3D{
					Vertex{5, 2, 0, 1.0},
					Vertex{4, 2, 2, 1.0},
					Vertex{7, 2, 3, 1.0},
					blue},
			Triangle3D{
					Vertex{5, 2, 0, 1.0},
					Vertex{7, 2, 3, 1.0},
					Vertex{6, 2, 1, 1.0},
					blue},
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
			Triangle3D{
					Vertex{4, 4, 0, 1.0},
					Vertex{5, 4, 2, 1.0},
					Vertex{1, 4, 3, 1.0},
					purple},
			Triangle3D{
					Vertex{4, 4, 0, 1.0},
					Vertex{1, 4, 3, 1.0},
					Vertex{0, 4, 1, 1.0},
					purple},
			Triangle3D{
					Vertex{2, 5, 0, 1.0},
					Vertex{6, 5, 2, 1.0},
					Vertex{7, 5, 3, 1.0},
					cyan},
			Triangle3D{
					Vertex{2, 5, 0, 1.0},
					Vertex{7, 5, 3, 1.0},
					Vertex{3, 5, 1, 1.0},
					cyan},
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

	return item;
}
