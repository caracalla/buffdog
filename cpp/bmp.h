#ifndef BUFFDOG_BMP
#define BUFFDOG_BMP

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "device.h"


#define DATA_OFFSET_OFFSET 0x000A
#define WIDTH_OFFSET 0x0012
#define HEIGHT_OFFSET 0x0016
#define BITS_PER_PIXEL_OFFSET 0x001C
#define HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define NO_COMPRESION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0


typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t bytes_per_pixel;
	unsigned char* pixel_buffer;
} bmp_info_t;

bmp_info_t readBMPFile(const char* filename) {
	bmp_info_t result;
	FILE *file = fopen(filename, "rb");

	if (!file) {
		char message[1024];
		snprintf(
				message,
				sizeof(message),
				"could't read bmp file %s\n",
				filename);

		terminateFatal(message);
	}

	// read offset
	uint32_t data_offset;
	fseek(file, DATA_OFFSET_OFFSET, SEEK_SET);
	fread(&data_offset, 4, 1, file);

	// read width
	fseek(file, WIDTH_OFFSET, SEEK_SET);
	fread(&result.width, 4, 1, file);

	// read height
	fseek(file, WIDTH_OFFSET, SEEK_SET);
	fread(&result.height, 4, 1, file);

	// read bits per pixel
	uint16_t bits_per_pixel;
	fseek(file, BITS_PER_PIXEL_OFFSET, SEEK_SET);
	fread(&bits_per_pixel, 2, 1, file);

	result.bytes_per_pixel = (uint32_t)bits_per_pixel / 8;

	// each row is padded to be a multiple of 4 bytes
	// calculate padded row size in bytes
	int padded_row_size =
			(int)(4 * ceil((float)result.width / 4.0f)) * result.bytes_per_pixel;

	// not interested in the padding bytes, sso just allocating memory for the
	// pixel data
	int unpadded_row_size = result.width * result.bytes_per_pixel;

	// total size of the pixel data in bytes
	int total_size = unpadded_row_size * result.height;
	result.pixel_buffer = (unsigned char*)malloc(total_size);

	// read pixel data row by row
	// rows are padded and stored bottom-up

	// point to the last row
	unsigned char* current_row_pointer =
			result.pixel_buffer + ((result.height - 1) * unpadded_row_size);

	for (int i = 0; i < result.height; i++) {
		// put file cursor at next row going up
		fseek(file, data_offset + (i * padded_row_size), SEEK_SET);
		// read only unpadded_row_size bytes
		fread(current_row_pointer, 1, unpadded_row_size, file);
		// move to the next row;
		current_row_pointer -= unpadded_row_size;
	}

	fclose(file);

	return result;
}

struct BMPTexture {
	bmp_info_t bmp_info;
	bool exists = false;

	uint32_t colorFromUV(double u, double v) {
		// if (u < -0.0001 || u > 1.0001 || v < -0.0001 || v > 1.0001) {
		// 	printf("got bad uv - u: %f, v: %f\n", u, v);
		// 	return 0;
		// }

		int x = (bmp_info.width - 1) * u;
		int y = (bmp_info.height - 1) * v;

		size_t index = (bmp_info.width * y + x) * bmp_info.bytes_per_pixel;

		unsigned char red = bmp_info.pixel_buffer[index];
		unsigned char green = bmp_info.pixel_buffer[index + 1];
		unsigned char blue = bmp_info.pixel_buffer[index + 2];

		uint32_t raw_color = (blue << 24) + (green << 16) + (red << 8);

		return raw_color;
	}

	static BMPTexture load(const char* filename) {
		BMPTexture result;

		result.bmp_info = readBMPFile(filename);
		result.exists = true;

		return result;
	}
};

#endif
