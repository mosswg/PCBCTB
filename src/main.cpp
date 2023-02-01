#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <iterator>

#include "ctb.h"
#include "elegoo_mars3.h"

const std::vector<uint8_t> make_square(const printer& printer_type, uint32_t rec_x, uint32_t rec_y) {
	uint32_t resolution_x = printer_type.get_resolution_x();
	uint32_t resolution_y = printer_type.get_resolution_y();
	std::vector<uint8_t> out(resolution_x * resolution_y, 0);
	for (int y = 0; y < rec_y; y++) {
		for (int x = 0; x < rec_x; x++) {
				out[x + (y * resolution_y)] = 255;
		}
	}
	return out;
}

int main(int argc, char** argv) {
	mars3 mars;

	ctb_t ctb_file(mars);

	std::vector<uint8_t> square = make_square(mars, 200, 300);
	ctb_file.append_layer_image(square, 100);

	std::vector<uint8_t> file_data = ctb_file.make_file();

	std::ofstream output_file("output.ctb", std::ios::binary);

	std::ostream_iterator<uint8_t> output_iterator(output_file);
	std::copy(std::begin(file_data), std::end(file_data), output_iterator);
}
