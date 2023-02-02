#include "rle.h"

#include <iostream>

/*
 * Encode a pixel in the rle15 format.
 *
 * It means that it encodes the pixel on 1 byte, with a possible run flag on
 * the lsb of the green channel, and if that flag is set, the next byte
 * encodes how many times that pixel is repeated.
 */
/*
 * Encode a pixel in the rle15 format.
 *
 * It means that it encodes the pixel on 1 byte, with a possible run flag on
 * the lsb of the green channel, and if that flag is set, the next byte
 * encodes how many times that pixel is repeated.
 */
uint8_t REPEAT_RGB15_MASK = 0x20;
void rle15::add_pixel(uint16_t color15, uint8_t repetition, std::vector<uint8_t>& out_buffer) {
	switch (repetition)
		{
		case 0:
			return;

		case 1:
			out_buffer.push_back((uint8_t) (color15 & ~REPEAT_RGB15_MASK));
			out_buffer.push_back((uint8_t) ((color15 & ~REPEAT_RGB15_MASK) >> 8));
			break;

		case 2:
			out_buffer.push_back((uint8_t) (color15 & ~REPEAT_RGB15_MASK));
			out_buffer.push_back((uint8_t) ((color15 & ~REPEAT_RGB15_MASK) >> 8));
			out_buffer.push_back((uint8_t) (color15 & ~REPEAT_RGB15_MASK));
			out_buffer.push_back((uint8_t) ((color15 & ~REPEAT_RGB15_MASK) >> 8));
			break;

		default:
			out_buffer.push_back((uint8_t) (color15 | REPEAT_RGB15_MASK));
			out_buffer.push_back((uint8_t) ((color15 | REPEAT_RGB15_MASK) >> 8));
			out_buffer.push_back((uint8_t) ((repetition - 1) | 0x3000));
			out_buffer.push_back((uint8_t) (((repetition - 1) | 0x3000) >> 8));
			break;
		}
}

/*
 * Encode raw bitmap `in_buffer` of length `in_buffer_size` to
 * the RLE15 encoding of ctb preview images in `out_buffer`.
 *
 * `out_buffer_size` will be set to the length of `out_buffer`.
 *
 * Returns non-zero in case of error.
 */
constexpr uint16_t RLE16_ENCODING_LIMIT = 0xFFF;
void rle15::encode(const std::vector<uint8_t>& in_buffer, std::vector<uint8_t>& out_buffer) {
	uint16_t color15 = 0;
	uint32_t repetition = 0;

	for (uint64_t i = 0; i < in_buffer.size(); i++) {
		uint16_t new_color15 = (in_buffer[i+2] >> 3) | ((in_buffer[i+1] >> 2) << 5) | ((in_buffer[i] >> 3) << 11);
		i += 2;

		if (new_color15 == color15) {
			repetition++;
			if (repetition == RLE16_ENCODING_LIMIT) {
				rle15::add_pixel(color15, repetition, out_buffer);
				repetition = 0;
				}
			}
		else {
			rle15::add_pixel(color15, repetition, out_buffer);
			color15 = new_color15;
			repetition = 1;
			}
		}

	rle15::add_pixel(color15, repetition, out_buffer);
}

/*
 * Averages the pixels of image to convert into the desired size.
 */
void rle_helper::resize_raw_image(const std::vector<uint8_t>& image, std::vector<uint8_t>& out_buffer, uint32_t image_resolution_x, uint32_t image_resolution_y, uint32_t desired_image_resolution_x, uint32_t desired_image_resolution_y) {
	uint32_t increment_y = image_resolution_y / desired_image_resolution_y;
	uint32_t increment_x = image_resolution_x / desired_image_resolution_x;
	uint32_t average_value;

	if (image.size() != image_resolution_x * image_resolution_y) {
		std::cerr << "ERROR: Invalid image size in resize_raw_image\n";
		exit(1);
	}

	if (out_buffer.size() != desired_image_resolution_x * desired_image_resolution_y * 3) {
		std::cerr << "ERROR: Invalid out buffer size in resize_raw_image\n";
		exit(1);
	}

	for (int y = 0, i = 0; y < image_resolution_y && i < desired_image_resolution_y; y += increment_y, i++) {
		for (int x = 0, j = 0; x < image_resolution_x && j < desired_image_resolution_x; x += increment_x, j++) {
			/// Average out the values of the
			average_value = 0;
			for (int avg_index_y = 0; avg_index_y < increment_y; avg_index_y++) {
				for (int avg_index_x = 0; avg_index_x < increment_x; avg_index_x++) {
					average_value += image[(avg_index_x + x) + ((y + avg_index_y) * image_resolution_x)];
				}
				average_value /= increment_x;
			}
			average_value /= increment_y;

			// Add 3 duplicate values for red green and blue.
			uint32_t out_buffer_index = (j + (i * desired_image_resolution_x)) * 3;
			out_buffer[out_buffer_index] = average_value;
			out_buffer[out_buffer_index + 1] = average_value;
			out_buffer[out_buffer_index + 2] = average_value;
		}
	}
 }


/*
 * All output pixels are 0 or 0xff no gradient.
 *
 * This is a very dumb function. It logic is:
 * If there is a value at a pixel set it to 0 otherwise set it to 0xff.
 * Thats it!
 */
void rle_helper::invert_image(std::vector<uint8_t>& image, uint32_t image_resolution_x, uint32_t image_resolution_y) {
	for (uint32_t y = 0; y < image_resolution_y; y++) {
		for (uint32_t x = 0; x < image_resolution_x; x++) {
			uint8_t* pixel = &image[x + (y * image_resolution_x)];

			if (*pixel) {
				*pixel = 0;
			}
			else {
				*pixel = 0xff;
			}
		}
	}
}



void rle1::add_pixel (uint8_t color, uint32_t stride, std::vector<uint8_t>& out_buffer) {
	if (stride == 0)
		return;

	if (stride > 1)
		color |= 0x80;

	out_buffer.push_back(color);

	if (stride <= 1)
		return;

	if (stride <= 0x7f)
		{
			out_buffer.push_back(stride);
			return;
		}

	if (stride <= 0x3fff)
		{
			out_buffer.push_back(0b10000000 | (stride >> 8));
			out_buffer.push_back(stride);
			return;
		}

	if (stride <= 0x1fffff)
		{
			out_buffer.push_back(0b11000000 | (stride >> 16));
			out_buffer.push_back((stride >> 8));
			out_buffer.push_back(stride);
			return;
		}

	if (stride <= 0xfffffff)
		{
			out_buffer.push_back(0b11100000 | (stride >> 24));
			out_buffer.push_back((stride >> 16));
			out_buffer.push_back((stride >> 8));
			out_buffer.push_back(stride);
			return;
		}
}

void rle1::encode(const std::vector<uint8_t>& in_buffer, std::vector<uint8_t>& out_buffer) {
	uint8_t color = UINT8_MAX >> 1;
	uint32_t stride = 0;


	for (uint64_t i = 0; i < in_buffer.size(); i++)
		{
			uint8_t grey7 = in_buffer[i] >> 1;
			if (grey7 == color)
				stride++;
			else
				{
					rle1::add_pixel(color, stride, out_buffer);
					color = grey7;
					stride = 1;
				}
		}

	rle1::add_pixel(color, stride, out_buffer);
}
