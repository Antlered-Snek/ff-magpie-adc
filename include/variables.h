#pragma once

#include <Arduino.h>

#define F_CPU 9600000UL
#define writeHIGH(pin)	PORTB|= (1<<pin)
#define writeLOW(pin)	PORTB&=~(1<<pin) 


const int BATT_1S_PIN	= A2;
const int BATT_2S_PIN	= A3;
const int SEND_PIN		= PB2;
const int BIT_DELAY = 104;		// 9600 Baud => 104us/bit


typedef struct {
	uint16_t cell1_mv;
	uint16_t cell2_mv;
} Battery_Voltage;

extern Battery_Voltage batt_volts;
extern Battery_Voltage last_sent_batt_volts;

extern volatile int tick_counter;
extern volatile bool initiate_data_send;

// int16_t cell1_readings[5];
// int16_t cell2_readings[5];