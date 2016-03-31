/* 	FLASH MEMORY SPACES FOR SAVED SETTINGS	*/	// SegB starts at 0x1080, lenght 0x0040, end 0x10BF (?)
#include <intyppes.h>
#include <msp430g2553.h>
#include "settings.h"


#define BSTARTADR				(int *) 0x1080
#define MEM_LED_CYC				*(uint16_t *) 0x1080		// 2 Byte address for values up to 65s
#define MEM_FAN_CYC				*(uint16_t *) 0x1082		// ^^
#define MEM_LED_PWM				*(uint8_t *) 0x1084		// 1 Byte address for 0-100 PWM val (Might bug out!)
#define MEM_FAN_PWM				*(uint8_t *) 0x1085		// ^^


void changeSettings(Settings *set, int i, int newval){ //Must be given the address of settings i.e. &Settings1
	switch (i){
		case 1:
			set->cycle_time_led = newval;
			break;
		case 2:
			set->cycle_time_fan = newval;
			break;
		case 3:
			set->pwm_max_led = newval;
			break;
		case 4:
			set->pwm_max_fan = newval;
			break;
		default:
			break;
	}
}

void settings2Mem(Settings *set) { //Must be given the address of settings i.e. &Settings1
	FCTL2 = FWKEY + FSSEL_1 + FN1; 	// use MCLK/3
	FCTL1 = FWKEY + ERASE; 			// set to erase flash segment
	FCTL3 = FWKEY;					// set LOCK to 0
	INTERRUPT_CYCLES_MEM = 0x00;		// dummy write to iCyc to init erasing seg
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

void mem2settings(Settings *set){
	set->cycle_time_led = MEM_LED_CYC;
	set->max_pwm_led = MEM_LED_PWM;
	set->cycle_time_fan = MEM_FAN_CYC;
	set->max_pwm_fan = MEM_FAN_PWM;
}



