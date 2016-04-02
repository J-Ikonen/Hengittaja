/* 	FLASH MEMORY SPACES FOR SAVED SETTINGS	*/	// SegB starts at 0x1080, lenght 0x0040, end 0x10BF (?)
#include <inttypes.h>
#include <msp430g2553.h>
#include "settings.h"
#include "uart.h"
#include "pwm.h"



void changeSettings(Settings *set, int i, int newval){ //Must be given the address of settings i.e. &Settings1
	switch (i){
		case 1:
			set->cycle_time_led = scaleValues(newval, i);
			setHelpers(set);
			uart_puts((char *)"LED cycle time set\r\n");
			break;
		case 2:
			set->cycle_time_fan = scaleValues(newval, i);
			setHelpers(set);
			uart_puts((char *)"Fan cycle time set\r\n");
			break;
		case 3:
			set->pwm_max_led = scaleValues(newval, i);
			setHelpers(set);
			uart_puts((char *)"LED max PWM value set\r\n");
			break;
		case 4:
			set->pwm_max_fan = scaleValues(newval, i);
			setHelpers(set);
			uart_puts((char *)"Fan max PWM value set\r\n");
			break;
		case 5:
			settings2Mem(set);
			uart_puts((char *)"Settings saved\r\n");
			break;
		case 6:
			mem2Settings(set);
			setHelpers(set);
			uart_puts((char *)"Fetced saved settings\r\n");
			break;
		default:
			break;
	}
}
void settingsDefault(Settings *set) {
	set->cycle_time_led = 500;
	set->pwm_max_led = 8000;
	set->cycle_time_fan = 500;
	set->pwm_max_fan = 8000;
	setHelpers(set);
}

void settings2Mem(Settings *set) { //Must be given the address of settings i.e. &Settings1
	FCTL2 = FWKEY + FSSEL_1 + FN1; 	// use MCLK/3
	FCTL1 = FWKEY + ERASE; 			// set to erase flash segment
	FCTL3 = FWKEY;					// set LOCK to 0
	MEM_LED_CYC = 0x00;		// dummy write to iCyc to init erasing seg
	while(FCTL3 & BUSY);
	FCTL1 = FWKEY + WRT;			// Set write bit

	/*Ready to write!*/

	while(MEM_LED_CYC+MEM_LED_PWM+MEM_FAN_CYC+MEM_FAN_PWM !=
	set->cycle_time_led+set->cycle_time_fan+set->pwm_max_led+set->pwm_max_fan){ //Write until we get it right
		MEM_LED_CYC = set->cycle_time_led;
		while(FCTL3 & BUSY);
		MEM_FAN_CYC = set->cycle_time_fan;
		while(FCTL3 & BUSY);
		MEM_LED_PWM = set->pwm_max_led;
		while(FCTL3 & BUSY);
		MEM_FAN_PWM = set->pwm_max_fan;
		while(FCTL3 & BUSY);
	}
	FCTL1 = FWKEY;					// Clear write bit
	FCTL3 = FWKEY + LOCK;			// set lock bit
}

void mem2Settings(Settings *set){
	set->cycle_time_led = MEM_LED_CYC;
	set->pwm_max_led = MEM_LED_PWM;
	set->cycle_time_fan = MEM_FAN_CYC;
	set->pwm_max_fan = MEM_FAN_PWM;
}

/* Calculate values used by timer 0 ISR */
void setHelpers(Settings *set) {
	set->inter_cycles_max_res = set->cycle_time_led / 2;
	set->pwm_step_led = set->pwm_max_led / set->inter_cycles_max_res;
	set->pwm_step_fan = set->pwm_max_fan / set->inter_cycles_max_res;
}

uint16_t scaleValues(uint16_t value, int i) {
	switch(i)  {
		case 1:
			return (TIMER0_FRQ / TIMER0_MAX_COUNT) * value / 10;

		case 2:
			return (TIMER0_FRQ / TIMER0_MAX_COUNT) * value / 10;

		case 3:
			return value * TIMER1_MAX_COUNT / 100;

		case 4:
			return value * TIMER1_MAX_COUNT / 100;

		default:
			return value;

	}
}




