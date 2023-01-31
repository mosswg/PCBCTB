#include "chitubox.h"

const uint16_t REPEATRGB15MASK = 0x20;
const uint16_t RLE16EncodingLimit = 0xFFF;

const uint32_t FILE_FORMAT_VERSION = 3;

const uint32_t MAGIC_CTB = 0x12FD0086; // 318570630

const float LAYER_HEIGHT_MM = 1;
const float EXPOSURE_S = 1;
const float BOTTOM_EXPOSURE_S = 1;
const float LIGHT_OFF_S = 1;
const uint32_t BOTTOM_LAYER_COUNT = 1;

const uint32_t BASE_HEADER_SIZE = 0x70;

const uint32_t IMAGE_HEADER_SIZE = 0x20;

const uint32_t CONFIG_EXTENTION_OFFSET = BASE_HEADER_SIZE;
const uint32_t CONFIG_EXTENTION_SIZE = 0x3c;

const uint32_t CONFIG_EXTENTION_2_OFFSET = CONFIG_EXTENTION_OFFSET + CONFIG_EXTENTION_SIZE;
const uint32_t CONFIG_EXTENTION_2_SIZE = 0x4c;

const uint32_t PREVIEW_PIXEL_SIZE = 2;

const uint32_t LARGE_PREVIEW_SIZE_X = 10;
const uint32_t LARGE_PREVIEW_SIZE_Y = 10;
const uint32_t LARGE_PREVIEW_DATA_SIZE = LARGE_PREVIEW_SIZE_X * LARGE_PREVIEW_SIZE_Y * PREVIEW_PIXEL_SIZE;

const uint32_t LARGE_PREVIEW_OFFSET = CONFIG_EXTENTION_2_OFFSET + CONFIG_EXTENTION_2_SIZE;
const uint32_t LARGE_PREVIEW_SIZE = LARGE_PREVIEW_DATA_SIZE + IMAGE_HEADER_SIZE;

const uint32_t LARGE_PREVIEW_DATA_OFFSET = LARGE_PREVIEW_OFFSET + IMAGE_HEADER_SIZE;

const uint32_t SMALL_PREVIEW_SIZE_X = 10;
const uint32_t SMALL_PREVIEW_SIZE_Y = 10;
const uint32_t SMALL_PREVIEW_DATA_SIZE = SMALL_PREVIEW_SIZE_X * SMALL_PREVIEW_SIZE_Y * PREVIEW_PIXEL_SIZE;

const uint32_t SMALL_PREVIEW_OFFSET = LARGE_PREVIEW_OFFSET + LARGE_PREVIEW_SIZE;
const uint32_t SMALL_PREVIEW_SIZE = SMALL_PREVIEW_DATA_SIZE + IMAGE_HEADER_SIZE;

const uint32_t SMALL_PREVIEW_DATA_OFFSET = SMALL_PREVIEW_OFFSET + IMAGE_HEADER_SIZE;

const uint32_t MACHINE_TYPE_OFFSET = SMALL_PREVIEW_OFFSET + SMALL_PREVIEW_SIZE;
const std::string MACHINE_TYPE = "PCBCTB  ";

const uint32_t LAYER_TABLE_OFFSET = MACHINE_TYPE_OFFSET + MACHINE_TYPE.size();

const uint32_t LAYER_HEADER_SIZE = 36;


const uint32_t LEVEL_SET_COUNT = 1;

const uint16_t PWM_LEVEL = 0xff;
const uint16_t BOTTOM_PWM_LEVEL = 0xff;


const float LIFT_DISTANCE_MM = 1;
const float LIFT_SPEED_MMPM = 1;
const float BOTTOM_LIFT_DISTANCE_MM = 1;
const float BOTTOM_LIFT_SPEED_MMPM = 1;
const float RETRACT_SPEED_MMPM = 1;

// Easter egg 1
const float RESIN_VOLUME_ML = (float)'p';
const float RESIN_MASS_G = (float)'c';
const float RESIN_COST = (float)'b';


// Easter egg 2
const uint32_t MYSTERIOUS_ID = (('s' << 24) + ('s' << 16) + ('o' << 8) + ('m'));


// As recommended by cbiffle
const uint32_t SOFTWARE_VERSION = 0x00306010;


const float PCB_EXPOSURE_TIME_S = 15;


void RleRGB15(std::vector<uint8_t>& raw_data, uint16_t color15, uint8_t rep) {
	switch (rep)
	{
		case 0:
			return;
		case 1:
			raw_data.push_back(color15 & ~REPEATRGB15MASK);
			raw_data.push_back((color15 & ~REPEATRGB15MASK) >> 8);
			break;
		case 2:
			for (int i = 0; i < 2; i++)
			{
				raw_data.push_back(color15 & ~REPEATRGB15MASK);
				raw_data.push_back((color15 & ~REPEATRGB15MASK) >> 8);
			}

			break;
		default:
			raw_data.push_back(color15 | REPEATRGB15MASK);
			raw_data.push_back((color15 | REPEATRGB15MASK) >> 8);
			raw_data.push_back((rep - 1) | 0x3000);
			raw_data.push_back(((rep - 1) | 0x3000) >> 8);
			break;
	}
}


std::vector<uint8_t> chitubox::encode(const std::vector<uint8_t>& image) {
	std::vector<uint8_t> output;
	encode(image, output);
	return output;
}

void chitubox::encode(const std::vector<uint8_t>& image, std::vector<uint8_t>& output) {
	uint16_t color15 = 0;
	uint8_t rep = 0;

	long image_length = image.size();

	int pixel = 0;
	while (pixel < image_length)
	{
		uint16_t ncolor15 = (image[pixel++] >> 3) | ((image[pixel++] >> 2) << 5) | ((image[pixel++] >> 3) << 11); // bgr

		if (ncolor15 == color15)
		{
			rep++;
			if (rep == RLE16EncodingLimit)
			{
				RleRGB15(output, color15, rep);
				rep = 0;
			}
		}
		else
		{
			RleRGB15(output, color15, rep);
			color15 = ncolor15;
			rep = 1;
		}
	}

	RleRGB15(output, color15, rep);
}


void append_to_output(std::vector<uint8_t>& output, uint32_t value) {
	output.push_back(value & 0xff);
	output.push_back((value >> 8) & 0xff);
	output.push_back((value >> 16) & 0xff);
	output.push_back((value >> 24) & 0xff);
}

void append_to_output(std::vector<uint8_t>& output, uint16_t value) {
	output.push_back(value & 0xff);
	output.push_back((value >> 8) & 0xff);
}

void append_to_output(std::vector<uint8_t>& output, float value) {
	// Convert the bytes of value into an integer while keeping them the same. THIS IS NOT CHANGING THE VALUE OF THE BYTES.
	uint32_t value_as_int = *reinterpret_cast<uint32_t*>(&value);
	output.push_back(value_as_int & 0xff);
	output.push_back((value_as_int >> 8) & 0xff);
	output.push_back((value_as_int >> 16) & 0xff);
	output.push_back((value_as_int >> 24) & 0xff);
}


void append_to_output(std::vector<uint8_t>& output, const std::string& value) {
	for (uint8_t ch : value) {
		output.push_back(ch);
	}
}

void append_to_output(std::vector<uint8_t>& output, const std::vector<uint8_t>& value) {
	for (uint8_t byte : value) {
		output.push_back(byte);
	}
}


void pad_output(std::vector<uint8_t>& output, uint32_t count) {
	for (uint32_t i = 0; i < count; i++) {
		// Pushing back i because i think this is ignored so why not.
		append_to_output(output, (uint32_t)0);
	}
}

// Add 'count' number of four byte zero value for header values
void zero_output(std::vector<uint8_t>& output, uint32_t count) {
	for (uint32_t i = 0; i < count; i++) {
		append_to_output(output, (uint32_t)0);
	}
}


// Add 'count' number of zero bytes
void zero_bytes_output(std::vector<uint8_t>& output, uint32_t count) {
	for (uint32_t i = 0; i < count; i++) {
		output.push_back(0);
	}
}

// Add 'count' number of white pixels for preview
void add_preview_bytes_to_output(std::vector<uint8_t>& output, uint32_t pixel_count) {
	for (uint32_t i = 0; i < pixel_count; i++) {
		output.push_back(0xDF);
		output.push_back(0xFF);
	}
}


void append_run_of_pixels_to_vector(uint8_t pixel, uint32_t number, std::vector<uint8_t>& out) {
	if (number >= (0b1 << 28)) {
		std::cerr << "TOO MANY PIXELS IN RUN :(\n";
	}

	// Ensure that pixel is 7-bit
	pixel = pixel & 0x7f;
	if (number == 1) {
		out.push_back(pixel);
		return;
	}
	out.push_back( 0b10000000 | pixel );
	// 7-bit run
	if (number < (0b1 << 7)) {
		out.push_back(number);
	}
	// 14-bit run
	else if (number < (0b1 << 14)) {
		out.push_back(0b10000000 | (number >> (14 - 6)));
		out.push_back(number & 0xff);
	}
	// 21-bit run
	else if (number < (0b1 << 21)) {
		out.push_back(0b11000000 | (number >> (21 - 5)));
		out.push_back((number >> 8) & 0xff);
		out.push_back(number & 0xff);
	}
	// 28-bit run
	else {
		out.push_back(0b11100000 | (number >> (28 - 4)));
		out.push_back((number >> 16) & 0xff);
		out.push_back((number >> 8) & 0xff);
		out.push_back(number & 0xff);
	}
}


std::vector<uint8_t> generate_rectangle_for_output(const printer& printer_type, uint32_t rec_x, uint32_t rec_y) {
	std::vector<uint8_t> out;
	uint32_t max_x_size = printer_type.get_resolution_x();
	uint32_t max_y_size = printer_type.get_resolution_y();

	uint32_t pixels = 0;

	if (rec_y > max_y_size) {
		rec_y = max_y_size;
	}

	for (uint32_t y_index = 0; y_index < rec_y; y_index++) {
		append_run_of_pixels_to_vector(0xff, rec_x, out);
		pixels += rec_x;
		append_run_of_pixels_to_vector(0, max_x_size - rec_x, out);
		std::cout << "mx - rx = " << max_x_size - rec_x << "\n";
		pixels += max_x_size - rec_x;
	}
	if (rec_y < max_y_size) {
		// Fill remaining lines with 0
		uint32_t remaining_pixels = max_x_size * (max_y_size - rec_y);
		append_run_of_pixels_to_vector(0, remaining_pixels, out);
		std::cout << "appending " << remaining_pixels << " pxls\n";
		std::cout << "total: " << pixels + remaining_pixels << " intended: " << max_x_size * max_y_size << "\n";
	}
	return out;
}


std::vector<uint8_t> chitubox::make_file(const printer& printer_type, const uint32_t x_res, const uint32_t y_res) {
	std::vector<uint8_t> out;
	//// HEADER
	/// Meta-info
	append_to_output(out, MAGIC_CTB);
	append_to_output(out, FILE_FORMAT_VERSION);

	/// Printer Physical size
	append_to_output(out, printer_type.get_bed_x());
	append_to_output(out, printer_type.get_bed_y());
	append_to_output(out, printer_type.get_bed_z());

	/// eight zero bytes
	zero_output(out, 2);

	/// print physical
	append_to_output(out, LAYER_HEIGHT_MM);
	append_to_output(out, LAYER_HEIGHT_MM);

	/// exposure
	append_to_output(out, EXPOSURE_S);
	append_to_output(out, BOTTOM_EXPOSURE_S);
	append_to_output(out, LIGHT_OFF_S);
	append_to_output(out, BOTTOM_LAYER_COUNT);

	/// printer screen size
	append_to_output(out, printer_type.get_resolution_x());
	append_to_output(out, printer_type.get_resolution_y());

	/// Offsets
	append_to_output(out, LARGE_PREVIEW_OFFSET);
	append_to_output(out, LAYER_TABLE_OFFSET);
	append_to_output(out, BOTTOM_LAYER_COUNT);
	append_to_output(out, SMALL_PREVIEW_OFFSET);

	// Print time
	append_to_output(out, BOTTOM_EXPOSURE_S * BOTTOM_LAYER_COUNT);

	// flipped or not?
	append_to_output(out, (uint32_t)1);

	// extention config
	append_to_output(out, CONFIG_EXTENTION_OFFSET);
	append_to_output(out, CONFIG_EXTENTION_SIZE);

	// Level set count
	append_to_output(out, LEVEL_SET_COUNT);

	// Power modulation
	append_to_output(out, PWM_LEVEL);
	append_to_output(out, BOTTOM_PWM_LEVEL);

	// Encyption (0 since not used)
	append_to_output(out, (uint32_t)0);

	// extention 2 config
	append_to_output(out, CONFIG_EXTENTION_2_OFFSET);
	append_to_output(out, CONFIG_EXTENTION_2_SIZE);


	//// Config extention 1
	// Lift Distances
	append_to_output(out, BOTTOM_LIFT_DISTANCE_MM);
	append_to_output(out, BOTTOM_LIFT_SPEED_MMPM);
	append_to_output(out, LIFT_DISTANCE_MM);
	append_to_output(out, LIFT_SPEED_MMPM);
	append_to_output(out, RETRACT_SPEED_MMPM);

	// Resin meta data
	append_to_output(out, RESIN_VOLUME_ML);
	append_to_output(out, RESIN_MASS_G);
	append_to_output(out, RESIN_COST);

	// Light off time
	append_to_output(out, LIGHT_OFF_S);
	append_to_output(out, LIGHT_OFF_S);

	// Bottom layer count (again?)
	append_to_output(out, BOTTOM_LAYER_COUNT);

	pad_output(out, 4);

	//// Config extention 2
	zero_output(out, 7);

	// Machine type
	append_to_output(out, MACHINE_TYPE_OFFSET);
	append_to_output(out, (uint32_t)MACHINE_TYPE.size());

	// Encryption type (0xf even though encryption isn't used (idk why))
	append_to_output(out, (uint32_t)0xf);

	// mysterious id (I have no idea what this is)
	append_to_output(out, MYSTERIOUS_ID);

	// Anti-alias level (unused)
	append_to_output(out, (uint32_t)0);

	// Software version
	append_to_output(out, SOFTWARE_VERSION);

	// 0x200??????
	append_to_output(out, (uint32_t)0x200);

	pad_output(out, 5);


	//// Large preview
	std::cout << "lph at " << out.size() << "\n";
	// Size
	append_to_output(out, LARGE_PREVIEW_SIZE_X);
	append_to_output(out, LARGE_PREVIEW_SIZE_Y);

	// Data info
	append_to_output(out, LARGE_PREVIEW_DATA_OFFSET);
	append_to_output(out, LARGE_PREVIEW_DATA_SIZE);
	std::cout << "lpds: " << LARGE_PREVIEW_DATA_SIZE << "\n";

	// Padding
	zero_output(out, 4);

	add_preview_bytes_to_output(out, LARGE_PREVIEW_SIZE_X * LARGE_PREVIEW_SIZE_Y);


	//// Small preview
	// Size
	append_to_output(out, SMALL_PREVIEW_SIZE_X);
	append_to_output(out, SMALL_PREVIEW_SIZE_Y);

	// Data info
	append_to_output(out, SMALL_PREVIEW_DATA_OFFSET);
	append_to_output(out, SMALL_PREVIEW_DATA_SIZE);

	// Padding
	zero_output(out, 4);

	add_preview_bytes_to_output(out, SMALL_PREVIEW_SIZE_X * SMALL_PREVIEW_SIZE_Y);

	//// Machine type
	append_to_output(out, MACHINE_TYPE);

	//// Layer Table

	//// First (only) layer

	uint32_t first_layer_offset = out.size();

	/// Put the build platform at the top - 10
	append_to_output(out, printer_type.get_bed_z() - 10);

	// Exposure time
	append_to_output(out, PCB_EXPOSURE_TIME_S);

	// Light off time
	append_to_output(out, LIGHT_OFF_S);

	// Data offset
	append_to_output(out, first_layer_offset + LAYER_HEADER_SIZE);

	std::vector<uint8_t> layer = generate_rectangle_for_output(printer_type, x_res, y_res);

	// Data size
	append_to_output(out, (uint32_t)layer.size());

	zero_output(out, 4);

	append_to_output(out, layer);

	return out;
}
