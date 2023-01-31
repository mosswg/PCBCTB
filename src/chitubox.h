#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <iostream>

#include "printer.h"


namespace chitubox {
	std::vector<uint8_t> make_file(const printer& printer_type, const uint32_t x_res, const uint32_t y_res);
	std::vector<uint8_t> encode(const std::vector<uint8_t>& image);
	void encode(const std::vector<uint8_t>& image, std::vector<uint8_t>& output);
}
