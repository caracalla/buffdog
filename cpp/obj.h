#ifndef BUFFDOG_OBJ
#define BUFFDOG_OBJ

#include <cstdlib>
#include <utility>

#include "model.h"
#include "vector.h"


#define MAXCHAR 1024

// assume a str is three decimal numbers with spaces and a newline at the end
Vector readVector(char* str, size_t index, bool is_point) {
	Vector result;
	char num[MAXCHAR];

	if (is_point) {
		result = Vector::point(0, 0, 0);
	} else {
		result = Vector::direction(0, 0, 0);
	}

	for (int i = 0; i < 3; i++) {
		while (isspace(str[index])) {
			index += 1;
		}

		size_t num_index = 0;

		while (!isspace(str[index])) {
			num[num_index] = str[index];
			index += 1;
			num_index += 1;
		}

		num[num_index] = 0;
		result.at(i) = atof(num);
	}

	return result;
}

std::pair<double, double> readUV(char* str, size_t index) {
	std::pair<double, double> result;
	char num[MAXCHAR];

	while (isspace(str[index])) {
		index += 1;
	}

	size_t num_index = 0;

	while (!isspace(str[index])) {
		num[num_index] = str[index];
		index += 1;
		num_index += 1;
	}

	num[num_index] = 0;
	result.first = atof(num);

	while (isspace(str[index])) {
		index += 1;
	}

	num_index = 0;

	while (!isspace(str[index])) {
		num[num_index] = str[index];
		index += 1;
		num_index += 1;
	}

	num[num_index] = 0;
	result.second = atof(num);

	return result;
}

Triangle3D readTriangle(char* str, size_t index) {
	Triangle3D result;
	char num[MAXCHAR];

	for (int i = 0; i < 3; i++) {
		// v/vt/vn
		while (isspace(str[index])) {
			index += 1;
		}

		Vertex& vertex = result.at(i);
		size_t num_index = 0;

		while (isdigit(str[index])) {
			num[num_index] = str[index];
			index += 1;
			num_index += 1;
		}

		num[num_index] = 0;
		vertex.index = atoi(num) - 1;

		if (str[index] == '/') {
			// vt
			index += 1;
			num_index = 0;

			while (isdigit(str[index])) {
				num[num_index] = str[index];
				index += 1;
				num_index += 1;
			}

			num[num_index] = 0;
			vertex.uv = atoi(num) - 1;

			if (str[index] == '/') {
				// vn
				index += 1;
				num_index = 0;

				while (isdigit(str[index])) {
					num[num_index] = str[index];
					index += 1;
					num_index += 1;
				}

				num[num_index] = 0;
				vertex.normal = atoi(num) - 1;
			}
		}

		vertex.light_intensity = 1.0;
	}

	for (int i = 0; i < 3; i++) {
		result.color.at(i) = 0.8;
	}

	return result;
}

Model parseOBJFile(const char* filename) {
	Model result;
	char str[MAXCHAR];

	FILE *file = fopen(filename, "r");

	if (!file) {
		char message[MAXCHAR];
		snprintf(
				message,
				sizeof(message),
				"could't read obj file %s\n",
				filename);

		terminateFatal(message);
	}

	while (fgets(str, MAXCHAR, file) != NULL) {
		size_t index = 0;

		if (str[index] == '\n' || str[index] == '#') {
			// do nothing
		} else if (str[index] == 'v') {
			index += 1;

			if (isspace(str[index])) {
				// vertex
				result.vertices.push_back(readVector(str, index + 1, true));
			} else if (str[index] == 'n') {
				// normal
				result.normals.push_back(readVector(str, index + 1, false));
			} else if (str[index] == 't') {
				// texture coordinate
				result.uvs.push_back(readUV(str, index + 1));
			} else {
				// I don't know, skip
			}
		} else if (str[index] == 'f') {
			// face
			result.triangles.push_back(readTriangle(str, index + 1));
		}
	}

	return result;
}

#endif
