#pragma once
#include <cstdint>
#include <vector>

namespace rle_helper {
  void resize_raw_image(const std::vector<uint8_t>& image, std::vector<uint8_t>& out_buffer, uint32_t image_resolution_x, uint32_t image_resolution_y, uint32_t desired_image_resolution_x, uint32_t desired_image_resolution_y);

  void invert_image(std::vector<uint8_t>& image, uint32_t image_resolution_x, uint32_t image_resolution_y);
}


namespace rle15 {

  void add_pixel(uint16_t color15, uint8_t repetition, std::vector<uint8_t>& out_buffer);

  void encode(const std::vector<uint8_t>& in_buffer, std::vector<uint8_t>& out_buffer);


}

namespace rle1 {

  void encode(const std::vector<uint8_t>& in_buffer, std::vector<uint8_t>& out_buffer);
  void add_pixel (uint8_t color, uint32_t stride, std::vector<uint8_t>& out_buffer);

}
