#include <Arduino.h>
#include "variables.h"
#include "functions.h"
// #include <I2CTinyBB.h>
// #include "i2c_slave.h"

void setup() {
	// Initialize Timer0 in CTC Mode 
	TCNT0  = 0;
	OCR0A  = 93;                            // Compare Match Register with 10ms period
	TCCR0A = (1 << WGM01);                  // CTC mode
	TCCR0B = (1 << CS02) | (1 << CS00);     // 1024 prescaler
	TIMSK0 = (1 << OCIE0A);                 // Enable timer compare interrupt
	TIFR0  = (1 << OCF0A);                  // Enable Output Compare A Match Flag
	sei();

	// Initialize ADC
	ADMUX  = (1 << REFS0);    // Set Vref to AVCC
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);	// Enable ADC and set prescaler to 128 (9.6MHz / 128 = 75kHz)
	DIDR0  = (1 << ADC3D) | (1 << ADC2D);	// Disable digital input for better ADC precision

	// Digital Output
	DDRB = (1 << SEND_PIN);
}


int main() {
	setup();

	while (true) {
		// Reading ADC
		uint16_t raw1 = adc_read(BATT_1S_PIN);
		uint16_t raw2 = adc_read(BATT_2S_PIN);
	
		batt_volts.cell1_mv = raw_to_millivolts(raw1, 100);
		batt_volts.cell2_mv = raw_to_millivolts(raw2, 47) - batt_volts.cell1_mv;
		
		// Send Data
		if (initiate_data_send) {
			// Ignore if there's no change in value
			const bool change_in_value  = (last_sent_batt_volts.cell1_mv != batt_volts.cell1_mv)
									or (last_sent_batt_volts.cell2_mv != batt_volts.cell2_mv);
			if (change_in_value) {
				send_battery_voltage(batt_volts.cell1_mv, batt_volts.cell2_mv);
				last_sent_batt_volts = batt_volts;
			}
			initiate_data_send = false;		// Reset
		}

	}
}


ISR(TIM0_COMPA_vect) {
    tick_counter++;
    if (tick_counter >= 50) { // Fires every 500ms for 10ms tick setup
		initiate_data_send = true;
		tick_counter = 0;
    }
}