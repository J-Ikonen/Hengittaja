/* 	FLASH MEMORY SPACES FOR SAVED SETTINGS	*/	// SegB starts at 0x1080, lenght 0x0040, end 0x10BF (?)

#include <inttypes.h>
#include <msp430g2553.h>
#include "settings.h"
#include "uart.h"
#include "pwm.h"
#include "main.h"


void changeSettings(Settings *set, int i, int newval, RunValues *rv){ //Must be given the address of settings i.e. &Settings1
	if(newval < 0) {
		uart_puts((char *)"Neg arvoja ei hyvaksyta\r\n");
	}
	switch (i){
		case 1:
			//if(newval > 600) {
			//	uart_puts((char *)"Yli minuutti??? ok\r\n");
			//}
			set->cycle_time = scaleValues(newval, 1);
			uart_puts((char *)"Syklin aika ok\r\n");
			break;
		case 2:
			if(newval * 80 < set->pwm_min_led) {
				uart_puts((char *)"Virhe - LED max < min\r\n");
			} else {
				set->pwm_max_led = scaleValues(newval, 2);
				uart_puts((char *)"LED max arvo ok\r\n");
			}
			break;
		case 3:
			if(newval * 80 > set->pwm_max_led) {
				uart_puts((char *)"Virhe - LED max < min\r\n");
			} else {
				set->pwm_min_led = scaleValues(newval, 2);
				uart_puts((char *)"LED min arvo ok\r\n");
			}
			break;
		case 4:
			if(newval * 80 < set->pwm_min_fan) {
				uart_puts((char *)"Virhe - tuuletin max < min\r\n");
			} else {
				set->pwm_max_fan = scaleValues(newval, 2);
				uart_puts((char *)"Tuuletin max arvo ok\r\n");
			}
			break;
		case 5:
			if(newval * 80 > set->pwm_max_fan) {
				uart_puts((char *)"Virhe - tuuletin max < min\r\n");
			} else {
				set->pwm_min_fan = scaleValues(newval, 2);
				uart_puts((char *)"Tuuletin min arvo ok\r\n");
			}
			break;
		case 6:
			settingsDefault(set);
			uart_puts((char *)"Oletus asetukset asetettu\r\n");
			break;
		case 9:
			if(settings2Mem(set) == -1) {
				uart_puts((char *)"Virhe asetusten tallennuksessa\r\n");
			} else
				uart_puts((char *)"Asetukset tallennettu\r\n");
			break;
		case 10:
			mem2Settings(set);
			uart_puts((char *)"Asetukset otettu käyttöön\r\n");
			break;
		default:
			break;
	}

	if(i != 9) {
		setHelpers(set);
		reset_run_values(rv); // Reset run values to restart cycle on changed settings
	}

}
void settingsDefault(Settings *set) {
	set->cycle_time = 100;
	set->pwm_max_led = 8000;
	set->pwm_max_fan = 8000;
	set->pwm_min_led = 0;
	set->pwm_min_fan = 0;
	setHelpers(set);
}

/* settings2Mem
 * Saves settings to flash
 * INPUT: Settings to be saved
 * RETURN: 0 if saved correctly, -1 if not
 */
int settings2Mem(Settings *set) { //Must be given the address of settings i.e. &Settings1
	int retVal = -1;
	FCTL2 = FWKEY + FSSEL_1 + FN1; 	// use MCLK/3
	FCTL1 = FWKEY + ERASE; 			// set to erase flash segment
	FCTL3 = FWKEY;					// set LOCK to 0
	MEM_CYC = 0x00;					// dummy write to iCyc to init erasing seg
	while(FCTL3 & BUSY);
	FCTL1 = FWKEY + WRT;			// Set write bit

	/*Ready to write!*/

	if(MEM_CYC+MEM_LED_MAX_PWM+MEM_FAN_MAX_PWM+MEM_LED_MIN_PWM+MEM_FAN_MIN_PWM	 ==
	set->cycle_time+set->pwm_max_led+set->pwm_max_fan+set->pwm_min_fan+set->pwm_min_led){
		retVal = 0;
	}
	FCTL1 = FWKEY;					// Clear write bit
	FCTL3 = FWKEY + LOCK;			// set lock bit
	return retVal;
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
	if(set->pwm_step_fan < 2 || set->pwm_step_led < 2) {
		uart_puts((char *)"Arvot voivat aiheuttaa virheita tai epatarkkuuksia!!!\r\n");
	}
	if(set->pwm_step_fan == 0) {
		uart_puts((char *)"Tuulettimen teho ei muutu!!!\r\n");
	} else if(set->pwm_step_led == 0) {
		uart_puts((char *)"Ledien teho ei muutu!!!\r\n");
	}

}


uint16_t scaleValues(uint16_t value, int i) {
	switch(i)  {
		case 1:
			return 2 * value; 		// /10 to scale back to s from 0.1s

		case 2:
			return value * 80;			// 0-100% power

		default:
			break;

	}
	return value;
}




