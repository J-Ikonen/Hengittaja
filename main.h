/*
 * main.h
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

/* Values for pwm and cycling, save in values usable by code
 * Used as maximum values
 * user input value ranges
 * cycle_time 	--- 1 = 0.1s 	(10-60000)
 * pwm_max 		--- 1 = 1%		(0-100)
 *
 * uint16 max about 65000 with timer defaults in pwm.c allows for max cycle of about 54 minutes
 */
typedef struct settings {
	volatile uint16_t cycle_time;
	volatile uint16_t pwm_max_led;
	volatile uint16_t pwm_max_fan;
	volatile uint16_t pwm_step_led;
	volatile uint16_t pwm_step_fan;

} Settings;

typedef struct run_values {
	volatile uint16_t inter_cycles;
	volatile uint16_t pwm_dc_led;
	volatile uint16_t pwm_dc_fan;
	volatile int dir;
	volatile int help_count;

} RunValues;


/* FUNCTION DECLARATIONS */
void board_setup(void);
void delay_cycles(volatile uint32_t i);
void get_bt_data(void);


#endif /* MAIN_H_ */
