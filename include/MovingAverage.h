#pragma once

#include <Arduino.h>

class Moving_Average_uint16_5 {
	private:
		uint16_t values[5];
		uint8_t oldest_index;
		static const uint8_t size = 5;
	public:
		Moving_Average_uint16_5();
		void insert_values(uint16_t value);
		uint16_t update_filter(uint16_t new_value);
};