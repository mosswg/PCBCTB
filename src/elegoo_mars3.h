#pragma once
#include "printer.h"

class mars3 : public printer {
	public:
	float get_bed_x() const override {
		return 143.43;
	}
	float get_bed_y() const override {
		return 89.6;
	}
	float get_bed_z() const override {
		return 175;
	}

	uint32_t get_resolution_x() const override {
		return 4098;
	}
	uint32_t get_resolution_y() const override {
		return 2560;
	}
};
