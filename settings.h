/*
 * settings.h
 *
 *  Created on: 31.3.2016
 *      Author: Joonas
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <inttypes.h>
#include <msp430g2553.h>
#include "settings.h"

typedef struct settings {
	uint16_t cycle_time_led;
	uint16_t cycle_time_fan;
	uint8_t pwm_max_led;
	uint8_t pwm_max_fan;

} Settings;


void changeSettings(Settings *set, int i, int newval);

void settings2Mem(Settings *set);

void mem2Settings(Settings *set);



#endif /* SETTINGS_H_ */
