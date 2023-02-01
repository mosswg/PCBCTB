#include "rle.h"

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
			out_buffer.push_back(color15 & ~REPEAT_RGB15_MASK);
			out_buffer.push_back((color15 & ~REPEAT_RGB15_MASK) >> 8);
			break;
		case 2:
			for (int i = 0; i < 2; i++)
			{
				out_buffer.push_back(color15 & ~REPEAT_RGB15_MASK);
				out_buffer.push_back((color15 & ~REPEAT_RGB15_MASK) >> 8);
			}

			break;
		default:
			out_buffer.push_back(color15 | REPEAT_RGB15_MASK);
			out_buffer.push_back((color15 | REPEAT_RGB15_MASK) >> 8);
			out_buffer.push_back((repetition - 1) | 0x3000);
			out_buffer.push_back(((repetition - 1) | 0x3000) >> 8);
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
void rle15::encode(uint8_t *in_buffer, uint64_t in_buffer_size, std::vector<uint8_t>& out_buffer) {
	uint16_t color15 = 0;
	uint32_t repetition = 0;

	for (uint64_t i = 0; i < in_buffer_size; i++)
		{
			uint16_t new_color15 = (in_buffer[i+2] >> 3) | ((in_buffer[i+1] >> 2) << 5) | ((in_buffer[i] >> 3) << 11);
			i += 2;

			if (new_color15 == color15)
				{
					repetition++;
					if (repetition == RLE16_ENCODING_LIMIT)
						{
							rle15::add_pixel(color15, repetition, out_buffer);
							repetition = 0;
						}
				}
			else
				{
					rle15::add_pixel(color15, repetition, out_buffer);
					color15 = new_color15;
					repetition = 1;
				}
		}

	rle15::add_pixel(color15, repetition, out_buffer);
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
