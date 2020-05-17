#ifndef BUFFDOG_PPM
#define BUFFDOG_PPM

#include <cstdlib>

#include "device.h"
#include "texture.h"
#include "vector.h"

#define MAXCHAR 1024


struct ppm_info_t {
	size_t width;
	size_t height;
	int max_value;
	Vector* colors;
};

Vector readColor(char* str, int max_value) {
	Vector result;
	char num[MAXCHAR];
	size_t index = 0;

	for (int i = 0; i < 3; i++) {
		while (isspace(str[index])) {
			index += 1;
		}

		size_t num_index = 0;

		while (isdigit(str[index])) {
			num[num_index] = str[index];
			index += 1;
			num_index += 1;
		}

		num[num_index] = 0;
		result.at(i) = (double)atoi(num) / max_value;
	}

	return result;
}

ppm_info_t readPPMFile(const char* filename) {
	ppm_info_t result;
	char str[MAXCHAR];

	FILE *file = fopen(filename, "r");

	if (!file) {
		char message[MAXCHAR];
		snprintf(
				message,
				sizeof(message),
				"could't read bmp file %s\n",
				filename);

		terminateFatal(message);
	}

	int line_number = 0;

	while(fgets(str, MAXCHAR, file) != NULL) {
		line_number += 1;
		size_t index = 0;

		// skip the first line, assume P3
		if (line_number == 1) {
			continue;
		}

		// second line has width and height
		if (line_number == 2) {
			char num[MAXCHAR];
			size_t num_index = 0;

			while (isdigit(str[index])) {
				num[num_index] = str[index];
				index += 1;
				num_index += 1;
			}

			num[num_index] = 0;
			result.width = atoi(num);

			while (isspace(str[index])) {
				index += 1;
			}

			num_index = 0;

			while (isdigit(str[index])) {
				num[num_index] = str[index];
				index += 1;
				num_index += 1;
			}

			num[num_index] = 0;
			result.height = atoi(num);

			continue;
		}

		// third line has max value
		if (line_number == 3) {
			char num[MAXCHAR];
			size_t num_index = 0;

			while (isdigit(str[index])) {
				num[num_index] = str[index];
				index += 1;
				num_index += 1;
			}

			num[num_index] = 0;
			result.max_value = atoi(num);

			continue;
		}

		if (line_number == 4) {
			// at this point we have the info we need
			// every subsequent line has three values, r g b
			result.colors = (Vector*)malloc(result.width * result.height * sizeof(Vector));
		}

		size_t color_index = line_number - 3;

		result.colors[color_index] = readColor(str, result.max_value);
	}

	fclose(file);

	return result;
}

struct PPMTexture : public Texture {
	ppm_info_t ppm_info;

	Vector vectorColorFromUV(double u, double v) override {
		int x = (ppm_info.width - 1) * u;
		int y = (ppm_info.height - 1) * (1.0 - v);

		return ppm_info.colors[ppm_info.width * y + x];
	}

	static PPMTexture load(const char* filename) {
		PPMTexture result;

		result.ppm_info = readPPMFile(filename);

		return result;
	}
};

#endif
