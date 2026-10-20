#include "functions.h"
#include "variables.h"

#include <Arduino.h>



// ADC Functions
uint16_t adc_read(uint8_t channel) {
	// Select ADC channel (0-7), clearing the old MUX bits and setting new ones
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07);
    
    // Optional Dummy conversion if you have an external 100nF capacitor
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    
    // Real conversion
    ADCSRA |= (1 << ADSC);          // Start Conversion
    while (ADCSRA & (1 << ADSC));   // Wait until conversion is finished
    
    return ADC;                     // ADC Macro simplifies reading the ADC
}


// Battery Functions
uint16_t raw_to_millivolts(uint16_t raw, uint8_t r2_kohm) {
	// #define ADC_MULTIPLIER  	3300 / 1023;
	// #define BATT_2S_MULTIPLIER	(47000 + 100000) / 47000  * ADC_MULTIPLIER;
	// #define BATT_1S_MULTIPLIER	(47000 + 100000) / 100000 * ADC_MULTIPLIER;
	const uint32_t vcc_mv = 3300;
	const uint32_t raw_max = 1023;
	constexpr uint32_t total_resistance_kohm = (100 + 47);

	uint32_t numerator		= static_cast<uint32_t>(raw) * vcc_mv * total_resistance_kohm;
	uint32_t denominator	= static_cast<uint32_t>(r2_kohm) * raw_max;
	
	uint32_t final_value_mv = (numerator + denominator/2) / (denominator);
	return final_value_mv;	// Rounded to the nearest integer from (A/B + 0.5)
}


// Serial Communication Functions
void send_byte(uint8_t b) {
  // Start bit (LOW)
  writeLOW(SEND_PIN);
  _delay_us(BIT_DELAY);

  // 8 Data bits (LSB first)
  for (int i = 0; i < 8; i++) {
    if (b & (1 << i))	{ writeHIGH(SEND_PIN); }
    else 				{ writeLOW(SEND_PIN);  }
    _delay_us(BIT_DELAY);
  }

  // Stop bit (HIGH)
  writeHIGH(SEND_PIN);
  _delay_us(BIT_DELAY);
}

void send_battery_voltage(int16_t cell1, int16_t cell2) {
  // Send sync header
  send_byte(0xAA);
  send_byte(0x55);

  // Send Cell 1 (split into 2 bytes)
  send_byte(cell1 & 0xFF);          // Low byte
  send_byte((cell1 >> 8) & 0xFF);   // High byte

  // Send Cell 2 (split into 2 bytes)
  send_byte(cell2 & 0xFF);          // Low byte
  send_byte((cell2 >> 8) & 0xFF);   // High byte
}



