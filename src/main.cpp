#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <iterator>

#include "chitubox.h"
#include "elegoo_mars3.h"

int main(int argc, char** argv) {
	mars3 mars;
	std::vector<uint8_t> file_data = chitubox::make_file(mars, 500, 200);

	std::ofstream output_file("output.ctb", std::ios::binary);

	std::ostream_iterator<uint8_t> output_iterator(output_file);
	std::copy(std::begin(file_data), std::end(file_data), output_iterator);
}
