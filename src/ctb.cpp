#include "ctb.h"

std::vector<uint8_t> ctb_t::make_file() {
	// Skip initial header (header is written last)
  std::vector<uint8_t> out(sizeof(this->headers), 0);

  // preallocate memory for headers
  this->write_large_preview(out);

  this->write_small_preview(out);

  this->write_print_config(out);

  this->headers.slicer_config_offset = out.size();

  // preallocate memory for slicer_config
  out.resize(out.size() + sizeof(this->slicer_config));

  this->write_machine_name(out);

  this->write_v4_disclaimer(out);

  this->write_v4_config(out);

  this->write_layers(out);

  this->write_slicer_config(out);

  this->write_headers(out);


  return out;
}


/*
 * Write the large preview in `file`, filling `ctb` by using `sl1`.
 *
 * The preview images in sl1 and ctb do not have the same size nor
 * aspect ratio, so we need to crop and resize them. The large preview
 * is converted from 800x480 to 400x300.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
void ctb_t::write_large_preview(std::vector<uint8_t>& out) {
	ctb_preview_t header;

	this->headers.large_preview_offset = out.size();

	out.resize(out.size() + sizeof(ctb_preview_t));

	header.image_offset = out.size();

	size_t in_buffer_size = 400 * 300 * 3;
	uint8_t in_buffer[in_buffer_size];

	size_t transform_buffer_size = 400 * 300 * 3;
	uint8_t transform_buffer[transform_buffer_size];

	// crop and resize
	// Verbatum stolen from https://github.com/oelmekki/sl1toctb  convert.c
	int y = 0;
	int x = -1;
	int out_y = 0;
	int out_x = -1;
	for (size_t i = 0; i < in_buffer_size; i+=4) {
		x++;
		if (x == 800)
			{
				x = 0;
				y++;
			}

		if (y % 2 > 0)
			continue;

		if (x % 2 > 0)
			continue;

		out_x++;
		if (out_x == 400)
			{
				out_x = 0;
				out_y++;
			}

		transform_buffer[(30+out_y)*1200+out_x*3] = in_buffer[i];
		transform_buffer[(30+out_y)*1200+out_x*3+1] = in_buffer[i+1];
		transform_buffer[(30+out_y)*1200+out_x*3+2] = in_buffer[i+2];
	}

	std::vector<uint8_t> preview_buffer;


	rle15::encode(transform_buffer, transform_buffer_size, preview_buffer);

	// Append
	out.insert(out.end(), preview_buffer.begin(), preview_buffer.end());

	header.image_length = preview_buffer.size();
	header.resolution_x = 400;
	header.resolution_y = 300;


	for (int i = this->headers.large_preview_offset, j = 0; j < sizeof(ctb_preview_t); i++, j++) {
		// Copy the bytes of header into out starting at large preview offset.
		out[i] = ((uint8_t*)&header)[j];
	}

	std::cout << "endoflp: " << out.size() << "\n";
}

	/*
 * Write the small preview in `file`, filling `ctb` by using `sl1`.
 *
 * The preview images in sl1 and ctb do not have the same size nor
 * aspect ratio, so we need to crop and resize them. The small preview
 * is converted from 400x400 to 200x125.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
void ctb_t::write_small_preview(std::vector<uint8_t>& out) {
	ctb_preview_t header;

	this->headers.small_preview_offset = out.size();

	out.resize(out.size() + sizeof(ctb_preview_t));

	header.image_offset = out.size();

	uint32_t desired_image_x = 200;
	uint32_t desired_image_y = 125;

	size_t in_buffer_size = desired_image_x * desired_image_y * 3;
	uint8_t in_buffer[in_buffer_size];

	size_t transform_buffer_size = desired_image_x * desired_image_y * 3;
	uint8_t transform_buffer[transform_buffer_size];

	// crop and resize
	// Verbatum stolen from https://github.com/oelmekki/sl1toctb  convert.c
	int y = 0;
	int x = -1;
	int out_y = 0;
	int out_x = -1;

	for (size_t i = 0; i < in_buffer_size; i+=4)
		{
			x++;
			if (x == 400)
				{
					x = 0;
					y++;
				}

			if (y % 2 > 0)
				continue;

			if (x % 2 > 0)
				continue;

			if (y < 75)
				continue;

			if (y > 325)
				continue;

			out_x++;
			if (out_x == 400)
				{
					out_x = 0;
					out_y++;
				}

			transform_buffer[out_y*1200+out_x*3] = in_buffer[i];
			transform_buffer[out_y*1200+out_x*3+1] = in_buffer[i+1];
			transform_buffer[out_y*1200+out_x*3+2] = in_buffer[i+2];
		}

	std::vector<uint8_t> preview_buffer;


	rle15::encode(transform_buffer, transform_buffer_size, preview_buffer);

	// Append
	out.insert(out.end(), preview_buffer.begin(), preview_buffer.end());

	header.image_length = preview_buffer.size();
	header.resolution_x = desired_image_x;
	header.resolution_y = desired_image_y;

	std::cout << "spo: " << std::hex << this->headers.small_preview_offset << std::endl;

	for (int i = this->headers.small_preview_offset, j = 0; j < sizeof(ctb_preview_t); i++, j++) {
		// Copy the bytes of header into out starting at small preview offset.
		out[i] = ((uint8_t*)&header)[j];
	}
}
/*
 * Write the print config part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
void ctb_t::write_print_config(std::vector<uint8_t>& out) {
  this->headers.print_config_offset = out.size();
  this->headers.print_config_size = sizeof (this->print_config);

	out.resize(out.size() + sizeof(this->print_config));

	for (int i = this->headers.print_config_offset, j = 0; j < sizeof(this->print_config); i++, j++) {
		// Copy the bytes of header into out starting at small preview offset.
		out[i] = ((uint8_t*)&this->print_config)[j];
	}
}

/*
 * Write the headers part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
void ctb_t::write_machine_name(std::vector<uint8_t>& out) {
	this->slicer_config.machine_type_offset = out.size();
	this->slicer_config.machine_type_len = this->machine_name.length();

	for (char ch : this->machine_name) {
		out.push_back(ch);
	}
}

/*
 * Write the headers part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
void ctb_t::write_v4_disclaimer(std::vector<uint8_t>& out) {
	this->print_config_v4.disclaimer_offset = out.size();
	this->print_config_v4.disclaimer_len = this->v4_disclaimer.length();

	std::cout << "cv4d at " << this->print_config_v4.disclaimer_offset << " len " << this->print_config_v4.disclaimer_len << "\n";

	for (char ch : this->v4_disclaimer) {
		out.push_back(ch);
	}
}

/*
 * Write the print_config_v4 part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
void ctb_t::write_v4_config(std::vector<uint8_t>& out) {
  this->slicer_config.print_config_v4_offset = out.size();

	std::cout << "cV4 at " << this->slicer_config.print_config_v4_offset << "\n";

	/// Defaults from sl12ctb
  this->print_config_v4.bottom_retract_speed = 210;
  this->print_config_v4.bottom_retract_speed2 = 0;
  this->print_config_v4.four1 = 4;
  this->print_config_v4.four2 = 4;
  this->print_config_v4.rest_time_after_retract = 0.5;
  this->print_config_v4.rest_time_after_lift = 0;
  this->print_config_v4.rest_time_before_lift = 0;
  this->print_config_v4.bottom_retract_height2 = 0;
  this->print_config_v4.unknown1 = 29008;
  this->print_config_v4.unknown2 = 267967;
  this->print_config_v4.unknown3 = 4;
  this->print_config_v4.last_layer_index = this->number_of_layers;

	out.resize(out.size() + sizeof(this->print_config_v4));

	std::cout << "btmretractspd: " << std::hex  << *((uint32_t*)&this->print_config_v4.bottom_retract_speed) << "\n";

	for (int i = this->slicer_config.print_config_v4_offset, j = 0; j < sizeof(this->print_config_v4); i++, j++) {
		// Copy the bytes of header into out starting at config v4 offset.
		out[i] = ((uint8_t*)&this->print_config_v4)[j];
	}
}

/*
 * Write the headers part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
void ctb_t::write_layers(std::vector<uint8_t>& out) {
  int err = 0;
  uint64_t layers_count = this->number_of_layers;
	std::vector<uint8_t> encoded_layer_buffer;

  this->headers.layer_table_offset = out.size();

  ctb_layer_header_base_t headers[layers_count];
	for (uint32_t i = 0; i < layers_count; i++) {
		ctb_layer_header_base_t header = {0};
		headers[i] = header;
	}


	out.resize(out.size() + (sizeof(ctb_layer_header_base_t) * layers_count));

	// write layers
	for (size_t i = 0; i < layers_count; i++) {
		int ext_pos = out.size();
		ctb_layer_header_extended_t ext = {0};

		out.resize(out.size() + (sizeof(ctb_layer_header_extended_t)));

		headers[i].data_offset = out.size();

		rle1::encode(this->layer_images[i], encoded_layer_buffer);

		std::cout << "encoded layer size: " << encoded_layer_buffer.size() << "\n";

		out.insert(out.end(), encoded_layer_buffer.begin(), encoded_layer_buffer.end());

		headers[i].data_len = encoded_layer_buffer.size();

		ext.z = layer_heights[i];
		ext.data_offset = headers[i].data_offset;
		ext.data_len = headers[i].data_len;
		ext.table_size = 84;
		ext.total_size = encoded_layer_buffer.size() + sizeof (ctb_layer_header_base_t);
		ext.lift_height = 7;
		ext.lift_speed = 70;
		ext.retract_speed = 210;
		ext.rest_time_after_retract = 0.5;
		ext.light_pwm = 255;

		headers[i].z = layer_heights[i];
		headers[i].table_size = 84;

		ext.exposure_time = PCB_EXPOSURE_TIME_S;
		headers[i].exposure_time = PCB_EXPOSURE_TIME_S;

		std::cout << "writting ext header to " << std::hex << ext_pos << "\n";
		std::cout << "data at: " << std::hex << headers[i].data_offset << std::endl;
		std::cout << "sizeof(ext_hdr): " << std::hex << sizeof(ext) << "\n";
		for (int i = ext_pos, j = 0; j < sizeof(ctb_layer_header_extended_t); i++, j++) {
			// Copy the bytes of header into out starting at ext position.
			out[i] = ((uint8_t*)&ext)[j];
		}

		/// Reset the buffer
		encoded_layer_buffer.clear();
	}

	int index = this->headers.layer_table_offset;
	for (size_t i = 0; i < layers_count; i++) {
		ctb_layer_header_base_t header = headers[i];

		for (int j = 0; j < sizeof(ctb_layer_header_base_t); index++, j++) {
			// Copy the bytes of header into out starting at index.
			out[index] = ((uint8_t*)&header)[j];
		}
	}
}

/*
 * Write the slicer config part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
void ctb_t::write_slicer_config(std::vector<uint8_t>& out) {
  this->headers.slicer_config_size = sizeof (this->slicer_config);

	// Default values of sl12ctb
  this->slicer_config.bottom_lift_height2 = 0;
  this->slicer_config.bottom_lift_speed2 = 0;
  this->slicer_config.lift_height2 = 0;
  this->slicer_config.lift_speed2 = 0;
  this->slicer_config.retract_distance2 = 0;
  this->slicer_config.retract_speed2 = 0;
  this->slicer_config.reset_time_after_lift = 0;
  this->slicer_config.per_layer_settings = 0x4000000F;
  this->slicer_config.mysterious_id = 27345357;
  this->slicer_config.antialias_level = 4;
  this->slicer_config.software_version = 17367040;
  this->slicer_config.rest_time_after_retract = 0.5;
  this->slicer_config.rest_time_after_lift2 = 0;
  this->slicer_config.transition_layer_count = 10;


	for (int j = 0, i = this->headers.slicer_config_offset; j < sizeof(ctb_slicer_config_t); j++, i++) {
		// Copy the bytes of header into out starting at the end of out.
		out[i] = (((uint8_t*)&this->slicer_config)[j]);
	}
}

/*
 * Write the headers part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
void ctb_t::write_headers(std::vector<uint8_t>& out) {
  int err = 0;

  this->headers.magic = 0x12fd0086;
  this->headers.version = 4;
  this->headers.bed_size_x = this->bed_x;
  this->headers.bed_size_y = this->bed_y;
  this->headers.bed_size_z = this->bed_z;
  this->headers.total_height = this->number_of_layers * this->headers.layer_height;
  this->headers.resolution_x = this->resolution_x;
  this->headers.resolution_y = this->resolution_y;
  this->headers.layer_count = this->number_of_layers;
  this->headers.print_time = 120;
  this->headers.projection = 1;
  this->headers.antialias_level = 1;
  this->headers.pwm_level = 255;
  this->headers.bottom_pwm_level = 255;


	for (int j = 0; j < sizeof(this->headers); j++) {
		// Copy the bytes of header into out starting at the start of out.
		out[j] = (((uint8_t*)&this->headers)[j]);
	}
}
