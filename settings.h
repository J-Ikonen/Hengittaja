/*
 * settings.h
 *
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_
#include "main.h"


#define BSTARTADR				(int *) 0x1080
#define MEM_CYC					*(uint16_t *) 0x1080		// 2 Byte address for values up to 65s
#define MEM_LED_MAX_PWM			*(uint16_t *) 0x1082		// 2 Byte address for 0-100 PWM val (Might bug out!)
#define MEM_FAN_MAX_PWM			*(uint16_t *) 0x1084		// ^^
#define MEM_LED_MIN_PWM			*(uint16_t *) 0x1086		// 2 Byte address for 0-100 PWM val (Might bug out!)
#define MEM_FAN_MIN_PWM			*(uint16_t *) 0x1088

/* Values for pwm and cycling, save in values usable by code
 * Used as maximum values
 * user input value ranges
 * cycle_time 	--- 1 = 0.1s 	(10-60000)
 * pwm_max 		--- 1 = 1%		(0-100)
 *
 * uint16 max about 65000 with timer defaults in pwm.c allows for max cycle of about 54 minutes
 */
/*
typedef struct settings {
	volatile uint16_t cycle_time;
	volatile uint16_t pwm_max_led;
	volatile uint16_t pwm_max_fan;
	volatile uint16_t pwm_step_led;
	volatile uint16_t pwm_step_fan;

} Settings;
*/

void changeSettings(Settings *set, int i, int newval, RunValues *rv);

int settings2Mem(Settings *set);

void mem2Settings(Settings *set);

void settingsDefault(Settings *set);

void setHelpers(Settings *set);

uint16_t scaleValues(uint16_t value, int i);

#endif /* SETTINGS_H_ */
