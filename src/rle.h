#pragma once
#include <cstdint>
#include <vector>

namespace rle15 {

  void add_pixel(uint16_t color15, uint8_t repetition, std::vector<uint8_t>& out_buffer);

  void encode(uint8_t *in_buffer, uint64_t in_buffer_size, std::vector<uint8_t>& out_buffer);

}

namespace rle1 {

  void encode(const std::vector<uint8_t>& in_buffer, std::vector<uint8_t>& out_buffer);
  void add_pixel (uint8_t color, uint32_t stride, std::vector<uint8_t>& out_buffer);

}
