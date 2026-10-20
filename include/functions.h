#pragma once

#include <Arduino.h>


// ADC Functions
uint16_t adc_read(uint8_t channel);

// Battery Functions
uint16_t raw_to_millivolts(uint16_t raw, uint8_t r2_kohm);

// Serial Communication Functions
void send_byte(uint8_t b);
void send_battery_voltage(int16_t cell1, int16_t cell2);
