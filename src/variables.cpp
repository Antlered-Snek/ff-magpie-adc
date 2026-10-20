#include "variables.h"
#include <Arduino.h>

volatile int tick_counter = 0;
volatile bool initiate_data_send = true;

Battery_Voltage batt_volts = {0};
Battery_Voltage last_sent_batt_volts = {0};