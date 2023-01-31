#pragma once
#include <cstdint>

class printer {
	public:
	virtual float get_bed_x() const = 0;
	virtual float get_bed_y() const = 0;
	virtual float get_bed_z() const = 0;

	virtual uint32_t get_resolution_x() const = 0;
	virtual uint32_t get_resolution_y() const = 0;
};
