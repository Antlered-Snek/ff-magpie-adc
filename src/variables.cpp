#include "variables.h"
#include <Arduino.h>

volatile uint8_t tick_counter = 0;
volatile bool initiate_data_send = true;

Battery_Voltage batt_volts = {0};
Battery_Voltage last_sent_batt_volts = {0};

Moving_Average_uint16_5 raw1_readings;
Moving_Average_uint16_5 raw2_readings;