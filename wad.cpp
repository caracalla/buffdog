#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "quake_types.h"
#include "util.h"


#define DUMP_TO_PPM 0


struct Color {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;

	void log() {
		printf("color: %d %d %d\n", this->red, this->green, this->blue);
	}
};


constexpr const char* k_input_file = "/Users/james/quake/wads/QUAKE101.WAD";
constexpr int k_palette_index = 84;


int main() {
	std::vector<unsigned char> raw_wad = util::readFile(k_input_file);

	wad_header_t* header = (wad_header_t*)raw_wad.data();

	printf("header->magic value: ");
	fwrite(header->magic, sizeof(char), 4, stdout);
	printf("\n");
	printf("header->numentries: %d\n", header->numentries);
	printf("header->diroffset: %d\n", header->diroffset);
	printf("size of file: %lu\n", raw_wad.size());

	wad_entry_t* entries = (wad_entry_t*)&(raw_wad.data()[header->diroffset]);

	// for (int i = 0; i < header->numentries; i++) {
	//   printf("entry %d: name - %s, size: %d\n", i, entries[i].name, entries[i].size);
	// }

	// palette at 84
	wad_entry_t palette = entries[k_palette_index];
	printf("palette: name - %s, size: %d, offset: %d\n", palette.name, palette.size, palette.offset);

#if DUMP_TO_PPM == 1
	// load the palette
	std::vector<Color> colors(256);
	unsigned char* palette_data = &(raw_wad.data()[palette.offset]) + 0;

	for (int i = 0; i < 256; i += 1) {
		int palette_index = i * 3;
		colors[i].red = palette_data[palette_index];
		colors[i].green = palette_data[palette_index + 1];
		colors[i].blue = palette_data[palette_index + 2];
		colors[i].alpha = 255;
	}

	// load each texture
	for (int i = 0; i < header->numentries; i++) {
		if (i == k_palette_index) {
			continue;
		}

		unsigned char* raw_tex_data = &(raw_wad.data()[entries[i].offset]);
		miptex_t* tex = (miptex_t*)raw_tex_data;

		char output_file[64];
		constexpr char const* k_ppm_location_format = "tex/%s.ppm";
		int result = sprintf(output_file, k_ppm_location_format, tex->name);

		if (result < 0) {
			throw std::runtime_error("failed to write output file name");
		}

		std::ofstream output;
		output.open(output_file);
		output << "P3\n" << tex->width << " " << tex->height << "\n255\n";

		for (int i = tex->offset1; i < tex->offset1 + (tex->width * tex->height); i++) {
			Color& color = colors[raw_tex_data[i]];
			output << (int)color.red << " " << (int)color.green << " " << (int)color.blue << "\n";
		}

		output.close();
	}
#endif

	return 0;
}
