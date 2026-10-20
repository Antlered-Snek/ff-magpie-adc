#include "MovingAverage.h"
#include <Arduino.h>

Moving_Average_uint16_5::Moving_Average_uint16_5() {
	oldest_index = 0;	
}

void Moving_Average_uint16_5::insert_values(uint16_t value) {
	for (int i=0; i<size; i++) { values[i] = value; }
}

uint16_t Moving_Average_uint16_5::update_filter(uint16_t new_value) {
	// Insert New Value
	values[oldest_index] = new_value;

	// Update Oldest Index
	oldest_index++;
	if (oldest_index >= size) { oldest_index = 0; }

	// Calculate new filtered value
	uint16_t sum = 0;
	uint16_t filtered_value;
	for (int i=0; i<size; i++) { sum += values[i]; }
	filtered_value = (sum + size/2) / size;		// Round to closest Integer (A/B + 0.5)
	
	return filtered_value;
}