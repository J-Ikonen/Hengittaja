/* 	FLASH MEMORY SPACES FOR SAVED SETTINGS	*/	// SegB starts at 0x1080, lenght 0x0040, end 0x10BF (?)

#include <inttypes.h>
#include <msp430g2553.h>
#include "settings.h"
#include "uart.h"
#include "pwm.h"
#include "main.h"


void changeSettings(Settings *set, int i, int newval, RunValues *rv){ //Must be given the address of settings i.e. &Settings1
	switch (i){
		case 1:
			set->cycle_time = scaleValues(newval, 1);
			uart_puts((char *)"Cycle time set\r\n");
			break;
		case 2:
			set->pwm_max_led = scaleValues(newval, 2);
			uart_puts((char *)"LED max PWM value set\r\n");
			break;
		case 3:
			set->pwm_min_led = scaleValues(newval, 2);
			uart_puts((char *)"LED min PWM value set\r\n");
			break;
		case 4:
			set->pwm_max_fan = scaleValues(newval, 2);
			uart_puts((char *)"Fan max PWM value set\r\n");
			break;
		case 5:
			set->pwm_min_fan = scaleValues(newval, 2);
			uart_puts((char *)"Fan min PWM value set\r\n");
			break;
		case 6:
			settingsDefault(set);
			break;
		case 9:
			settings2Mem(set);
			uart_puts((char *)"Settings saved\r\n");
			break;
		case 10:
			mem2Settings(set);
			uart_puts((char *)"Fetced saved settings\r\n");
			break;
		default:
			break;
	}
	if(i != 5) {
		setHelpers(set);
		reset_run_values(rv); // Reset run values to restart cycle on changed settings
	}

}
void settingsDefault(Settings *set) {
	set->cycle_time = 200;
	set->pwm_max_led = 8000;
	set->pwm_max_fan = 8000;
	set->pwm_min_led = 0;
	set->pwm_min_fan = 0;
	setHelpers(set);
}

void settings2Mem(Settings *set) { //Must be given the address of settings i.e. &Settings1
	FCTL2 = FWKEY + FSSEL_1 + FN1; 	// use MCLK/3
	FCTL1 = FWKEY + ERASE; 			// set to erase flash segment
	FCTL3 = FWKEY;					// set LOCK to 0
	MEM_CYC = 0x00;					// dummy write to iCyc to init erasing seg
	while(FCTL3 & BUSY);
	FCTL1 = FWKEY + WRT;			// Set write bit

	/*Ready to write!*/

	while(MEM_CYC+MEM_LED_MAX_PWM+MEM_FAN_MAX_PWM+MEM_LED_MIN_PWM+MEM_FAN_MIN_PWM	 !=
	set->cycle_time+set->pwm_max_led+set->pwm_max_fan){ //Write until we get it right
		MEM_CYC = set->cycle_time;
		while(FCTL3 & BUSY);
		MEM_LED_MAX_PWM = set->pwm_max_led;
		while(FCTL3 & BUSY);
		MEM_FAN_MAX_PWM = set->pwm_max_fan;
		while(FCTL3 & BUSY);
		MEM_LED_MIN_PWM = set->pwm_min_led;
		while(FCTL3 & BUSY);
		MEM_FAN_MIN_PWM = set->pwm_min_fan;
		while(FCTL3 & BUSY);
	}
	FCTL1 = FWKEY;					// Clear write bit
	FCTL3 = FWKEY + LOCK;			// set lock bit
}

void mem2Settings(Settings *set){
	set->cycle_time = MEM_CYC;
	set->pwm_max_led = MEM_LED_MAX_PWM;
	set->pwm_max_fan = MEM_FAN_MAX_PWM;
	set->pwm_min_led = MEM_LED_MIN_PWM;
	set->pwm_min_fan = MEM_FAN_MIN_PWM;
	setHelpers(set);
}

/* Calculate values used by timer 0 ISR */
void setHelpers(Settings *set) {
	set->pwm_step_led = (set->pwm_max_led - set->pwm_min_led)/ (set->cycle_time / 2);
	set->pwm_step_fan = (set->pwm_max_fan - set->pwm_min_fan)/ (set->cycle_time / 2);
}


uint16_t scaleValues(uint16_t value, int i) {
	switch(i)  {
		case 1:
			return 20 * value / 10; 		// /10 to scale back to s from 0.1s

		case 2:
			return value * 8000 / 100;			// 0-100% power


		default:
			break;

	}
	return value;
}




