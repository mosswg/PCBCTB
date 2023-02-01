#pragma once
/// Shamelessly stolen from https://github.com/oelmekki/sl1toctb  parser.h

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

#include "rle.h"
#include "printer.h"


const float LIGHT_OFF_S = 1;

const float PCB_EXPOSURE_TIME_S = 15;

const uint32_t DEFAULT_NUMBER_OF_LAYERS = 1;

typedef struct {
	uint32_t magic = 0x12fd0086;                   // 0x12fd0086 for ctb files.
	uint32_t version = 4;                 // ctb file version. We support 3 and 4.
	float bed_size_x;                 // length of the build plate, in mm.
	float bed_size_y;                 // width of the build plate, in mm.
	float bed_size_z;                 // height of the build plate, in mm.
	uint32_t unknown1 = 0;
	uint32_t unknown2 = 0;
	float total_height;               // height of the printed object, in mm.
	float layer_height = 1;               // height of each layer, in mm. Overriden in layer information.
	float layer_exposure = 1;             // exposure time for normal layers, in secs. Overriden in layer information.
	float bottom_exposure = 1;            // exposure time for bottom layers, in secs. Overriden in layer information.
	float light_off_delay;            // light off time setting used at slicing, in secs. Overriden in layer information.
																		// Also in ext_config.
	uint32_t bottom_layer_count;      // Number of layer being considered bottom. Also in ext_config.
	uint32_t resolution_x;            // Printer resolution on x axis, in px.
	uint32_t resolution_y;            // Printer resolution on y axis, in px.
	uint32_t large_preview_offset;    // offset in file for large preview image headers.
	uint32_t layer_table_offset;      // offset in file for layer headers.
	uint32_t layer_count;             // number of layers in the print.
	uint32_t small_preview_offset;    // offset in file for small preview image headers.
	uint32_t print_time = 1;              // estimated print time, in secs.
	uint32_t projection = 1;              // 0 (normal) or 1 (mirrored)
	uint32_t print_config_offset;     // offset in file for extended configuration.
	uint32_t print_config_size;       // size of the extended configuration, in bytes.
	uint32_t antialias_level;         // unused (legacy from cbddlp). Set to 1.
	uint16_t pwm_level = 0xff;               // PWM duty cycle for the UV illumination source on normal level.
																		// This appears to be an 8-bit quantity where 0xFF is fully on and 0x00 is fully off.
	uint16_t bottom_pwm_level = 0xff;        // PWM duty cycle for the UV illumination source on bottom level.
	uint32_t encryption_key = 0;          // Random value, used as seed for encryption. 0 since unused.
	uint32_t slicer_config_offset;    // offset in file for second extended configuration.
	uint32_t slicer_config_size;      // size of the second extended configuration, in bytes.
} ctb_headers_t;

typedef struct {
	float bottom_lift_height = 1;         // distance to lift the build platform away from the vat for bottom layers, in mm.
	float bottom_lift_speed = 1;          // speed at which to lift the build platform away from the vat after bottom layer, in mm/min
	float lift_height = 1;                // distance to lift the build platform away from the vat, in mm.
	float lift_speed = 1;                 // speed at which to lift the build platform away from the vat, in mm/min.
	float retract_speed = 1;              // speed to use when the build platform re-approaches the vat after lift, mm/min.
	float resin_volume = (float)'p';               // estimated resin needed, in ml.
	float resin_mass = (float)'c';                 // estimated resin needed, in g.
	float resin_cost = (float)'b';                 // estimated resin needed, in your currency.
	float bottom_light_off_delay;     // light off time setting used at slicing, in secs for bottom layers.
																		// Overriden in layer information.
	float light_off_delay;            // light off time setting used at slicing, in secs. Overriden in layer information.
																		// Also in ext_config.
	uint32_t bottom_layer_count;      // Number of layer being considered bottom. Also in ext_config.
	uint32_t padding1;
	uint32_t padding2;
	uint32_t padding3;
	uint32_t padding4;
} ctb_print_config_t;

typedef struct {
	float bottom_lift_height2;
	float bottom_lift_speed2;
	float lift_height2;
	float lift_speed2;
	float retract_distance2;
	float retract_speed2;
	float reset_time_after_lift;
	uint32_t machine_type_offset;     // file offset to a string naming the machine type.
	uint32_t machine_type_len;        // the length of the name string (not null terminated).
	uint32_t per_layer_settings;      // 0xF = per layer settings disabled, 0x2000000F = enabled (v3), 0x4000000F = enabled (v4).
	uint32_t mysterious_id = (('s' << 24) + ('s' << 16) + ('o' << 8) + ('m'));
	uint32_t antialias_level;
	uint32_t software_version = 0x00306010;
	float rest_time_after_retract;
	float rest_time_after_lift2;
	uint32_t transition_layer_count;
	uint32_t print_config_v4_offset;
	uint32_t padding1;
	uint32_t padding2;
} ctb_slicer_config_t;

typedef struct {
	float bottom_retract_speed;
	float bottom_retract_speed2;
	uint32_t padding1;
	float four1;
	uint32_t padding2;
	float four2;
	float rest_time_after_retract;
	float rest_time_after_lift;
	float rest_time_before_lift;
	float bottom_retract_height2;
	float unknown1;
	uint32_t unknown2;
	uint32_t unknown3;
	uint32_t last_layer_index;
	uint32_t padding3;
	uint32_t padding4;
	uint32_t padding5;
	uint32_t padding6;
	uint32_t disclaimer_offset;
	uint32_t disclaimer_len;
	uint32_t reserved[96];
} ctb_print_config_v4_t;

typedef struct {
	uint32_t resolution_x;
	uint32_t resolution_y;
	uint32_t image_offset;
	uint32_t image_length;
	uint32_t unknown1;
	uint32_t unknown2;
	uint32_t unknown3;
	uint32_t unknown4;
} ctb_preview_t;

typedef struct {
	float z;
	float exposure_time = PCB_EXPOSURE_TIME_S;  // in seconds
	float light_off_delay = LIGHT_OFF_S; // in seconds
	uint32_t data_offset;
	uint32_t data_len;
	uint32_t unknown1;
	uint32_t table_size;
	uint32_t unknown2;
	uint32_t unknown3;
} ctb_layer_header_base_t;

typedef struct {
	float z;
	float exposure_time = PCB_EXPOSURE_TIME_S;  // in seconds
	float light_off_delay = LIGHT_OFF_S; // in seconds
	uint32_t data_offset;
	uint32_t data_len;
	uint32_t unknown1;
	uint32_t table_size;
	uint32_t unknown2;
	uint32_t unknown3;

	uint32_t total_size;
	float lift_height;
	float lift_speed;
	float lift_height2;
	float lift_speed2;
	float retract_speed;
	float retract_height2;
	float retract_speed2;
	float rest_time_before_lift;
	float rest_time_after_lift;
	float rest_time_after_retract;
	float light_pwm;
} ctb_layer_header_extended_t;

typedef struct {
	ctb_layer_header_base_t base;
	ctb_layer_header_extended_t extended;
} ctb_layer_header_t;

class ctb_t {
	// file structure
	ctb_headers_t headers;
	ctb_preview_t large_preview;
	ctb_preview_t small_preview;
	ctb_print_config_t print_config;
	ctb_slicer_config_t slicer_config;
	std::string machine_name = "PCBCTB  ";
	std::string v4_disclaimer = "Layout and record format for the ctb and cbddlp file types are the copyrighted programs or codes of CBD Technology (China) Inc..The Customer or User shall not in any manner reproduce, distribute, modify, decompile, disassemble, decrypt, extract, reverse engineer, lease, assign, or sublicense the said programs or codes.";
	ctb_print_config_v4_t print_config_v4;
	std::vector<ctb_layer_header_t> layer_headers;

	uint32_t number_of_layers = 0;
	std::vector<std::vector<uint8_t>> layer_images;
	std::vector<float> layer_heights;
	float bed_x, bed_y, bed_z;
	uint32_t resolution_x, resolution_y;

	// not in the file
	std::string file_path;

	public:
	ctb_t(const printer& printer_type) {
		this->bed_x = printer_type.get_bed_x();
		this->bed_y = printer_type.get_bed_y();
		this->bed_z = printer_type.get_bed_z();

		this->resolution_x = printer_type.get_resolution_x();
		this->resolution_y = printer_type.get_resolution_y();
	}

	inline void append_layer_image(std::vector<uint8_t>& image, float height_mm) {
		this->layer_images.push_back(image);
		this->layer_heights.push_back(height_mm);
		this->number_of_layers++;
	}

	std::vector<uint8_t> make_file();

	void write_large_preview(std::vector<uint8_t>& out);

	void write_small_preview(std::vector<uint8_t>& out);

	void write_print_config(std::vector<uint8_t>& out);

	void write_machine_name(std::vector<uint8_t>& out);

	void write_v4_disclaimer(std::vector<uint8_t>& out);

	void write_v4_config(std::vector<uint8_t>& out);

	void write_layers(std::vector<uint8_t>& out);

	void write_slicer_config(std::vector<uint8_t>& out);

	void write_headers(std::vector<uint8_t>& out);
};
