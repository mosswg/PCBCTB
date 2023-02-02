#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>

// https://github.com/sammycage/lunasvg
#include <lunasvg.h>

#include "ctb.h"
#include "elegoo_mars3.h"

const std::vector<uint8_t> make_square(const printer& printer_type, uint32_t rec_x, uint32_t rec_y) {
	uint32_t resolution_x = printer_type.get_resolution_x();
	uint32_t resolution_y = printer_type.get_resolution_y();
	std::vector<uint8_t> out(resolution_x * resolution_y, 0);
	for (uint64_t y = 0; y < rec_y; y++) {
		for (uint64_t x = 0; x < rec_x; x++) {
			uint64_t index = x + (y * resolution_x);
			out[index] = 255;
		}
	}

	return out;
}

const std::vector<uint8_t> convert_to_monochromatic(const lunasvg::Bitmap& image, uint32_t output_resolution_x, uint32_t output_resolution_y) {
	std::vector<uint8_t> out(output_resolution_x * output_resolution_y, 0);
	const uint32_t bytes_per_pixel = 4;

	for (int image_y = 0; image_y < image.height(); image_y++) {
		for (int image_x = 0; image_x < image.width() * 4; image_x += bytes_per_pixel) {
			// Average RGB then multiply by 255 and divide by alpha.
			uint64_t average_value = 0;
			uint64_t pixel_index = (image_x + (image_y * (image.width() * bytes_per_pixel)));
			uint8_t r = image.data()[pixel_index], g = image.data()[pixel_index + 1], b = image.data()[pixel_index + 2], a = image.data()[pixel_index + 3];

			average_value = (r + g + b) / 3;

			uint8_t out_pixel = 0;
			if (a != 0) {
				if (average_value) {
					out_pixel = (average_value * 0xff) / a;
				}
				else {
					// If we don't have rgb but we do have alpha set the output to alpha.
					out_pixel = a;
				}
			}
			out[( image_x / bytes_per_pixel ) + (image_y * output_resolution_x)] = out_pixel;
		}
	}

	return out;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Usage: PCBCTB <svg-file>" << "\n";
		exit(1);
	}

	bool invert = false;
	if (argc > 2) {
		if (std::string(argv[2]) == "-i") {
			invert = true;
		}
	}

	auto document = lunasvg::Document::loadFromFile(argv[1]);
	if(!document) {
		std::cerr << "Could not open: " << argv[1] << "\n";
		exit(1);
	}
	auto bitmap = document->renderToBitmap(document->width() * 7.5);
	if(!bitmap.valid()) {
		std::cerr << "Could not render: " << argv[1] << "\n";
		exit(1);
	}

	bitmap.convertToRGBA();

	mars3 mars;

	std::vector<uint8_t> svg_mono = convert_to_monochromatic(bitmap, mars.get_resolution_x(), mars.get_resolution_y());

	ctb_t ctb_file(mars);

	ctb_file.append_layer_image(svg_mono, 5, invert);

	std::vector<uint8_t> file_data = ctb_file.make_file();

	std::ofstream output_file("output.ctb", std::ios::binary);

	std::ostream_iterator<uint8_t> output_iterator(output_file);
	std::copy(std::begin(file_data), std::end(file_data), output_iterator);
}
