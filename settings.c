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
		case 1:			// CYCLE TIME
			//if(newval > 600) {
			//	uart_puts((char *)"Yli minuutti??? ok\r\n");
			//}
			set->cycle_time = scaleValues(newval, 1);
			//uart_puts((char *)"Syklin aika ok\r\n");
			break;
		case 2:			// LED PWM MAX
			if(newval * PWM_SCALE_VAL < set->pwm_min_led) {
				uart_puts((char *)"Virhe - LED max < min\r\n");
			} else {
				set->pwm_max_led = scaleValues(newval, 2);
				//uart_puts((char *)"LED max arvo ok\r\n");
			}
			break;
		case 3:			// LED PWM MIN
			if(newval * PWM_SCALE_VAL > set->pwm_max_led) {
				//uart_puts((char *)"Virhe - LED max < min\r\n");
			} else {
				set->pwm_min_led = scaleValues(newval, 2);
				//uart_puts((char *)"LED min arvo ok\r\n");
			}
			break;
		case 4:			// FAN PWM MAX
			if(newval * PWM_SCALE_VAL < set->pwm_min_fan) {
				//uart_puts((char *)"Virhe - tuuletin max < min\r\n");
			} else {
				set->pwm_max_fan = scaleValues(newval, 2);
				//uart_puts((char *)"Tuuletin max arvo ok\r\n");
			}
			break;
		case 5:			// FAN PWM MIN
			if(newval * PWM_SCALE_VAL > set->pwm_max_fan) {
				//uart_puts((char *)"Virhe - tuuletin max < min\r\n");
			} else {
				set->pwm_min_fan = scaleValues(newval, 2);
				//uart_puts((char *)"Tuuletin min arvo ok\r\n");
			}
			break;
		case 6:			// FAN OUT OFF
			if(newval != 0 && newval != 1) {
				//uart_puts((char *)"Virhe - valitse 0 tai 1 arvoksi\r\n");
			} else {
				set->fan_out_off = newval;
				//uart_puts((char *)"Tuuletin uloshengitys ok\r\n");
			}
			break;
		case 7:			// CYCLE FORM
			if(newval != 0 && newval != 1) {
				//uart_puts((char *)"Virhe - valitse 0 tai 1 arvoksi\r\n");
			} else {
				set->cycle_form = newval;
				//uart_puts((char *)"Hengitysmuoto ok\r\n");
			}
			break;
		case 9:			// DEFAULT SETTINGS
			settingsDefault(set);
			uart_puts((char *)"Oletus asetukset asetettu\r\n");
			break;
		case 10:		// SAVE SETTINGS
			if(settings2Mem(set) == -1) {
				uart_puts((char *)"Virhe asetusten tallennuksessa\r\n");
			} else
				uart_puts((char *)"Asetukset tallennettu\r\n");
			break;
		case 11:		// LOAD SETTINGS
			mem2Settings(set);
			uart_puts((char *)"Asetukset otettu kayttoon\r\n");
			break;
		default:
			break;
	}

	if(i != 10) {
		setHelpers(set);
		reset_run_values(rv); // Reset run values to restart cycle on changed settings
	}

}

/* printHelp
 * Print all available commands and disable timer0 interrupts while printing
 */
void printHelp(void) {
	TA0CCTL0 |= CCIE;

	uart_puts((char *)"Mahdolliset toiminnot:\n");
	__delay_cycles(10000000);
	uart_puts((char *)"Lisaa '&' kaskyjen valiin jos samalla rivilla.\n");
	__delay_cycles(10000000);
	uart_puts((char *)"'1:x' Syklin aika (0.1s)\n");
	uart_puts((char *)"'2:x' Led max teho (%)\n");
	__delay_cycles(10000000);
	uart_puts((char *)"'3:x' Led min teho (%)\n");
	uart_puts((char *)"'4:x' Fan max teho (%)\n");
	__delay_cycles(10000000);
	uart_puts((char *)"'5:x' Fan min teho (%)\n");
	__delay_cycles(10000000);
	uart_puts((char *)"'6:x' Fan ei paalla uloshengityksessa? (0 || 1)\n");
	__delay_cycles(10000000);
	uart_puts((char *)"'7:' Hengityksen muoto (0-1)\n");
	uart_puts((char *)"'9:' Aseta oletus asetukset\n");
	__delay_cycles(10000000);
	uart_puts((char *)"'10:' Tallenna asetukset\n");
	uart_puts((char *)"'11:' Lataa asetukset\n");
	TA0CCTL0 |= CCIE;
}

void settingsDefault(Settings *set) {
	set->cycle_time = 800;
	set->pwm_max_led = 32000;
	set->pwm_max_fan = 32000;
	set->pwm_min_led = 0;
	set->pwm_min_fan = 0;
	set->fan_out_off = 1;
	set->cycle_form = 1;
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
	/* Write one setting and wait until ready again */
	MEM_CYC = set->cycle_time;
	while(FCTL3 & BUSY);
	MEM_LED_MAX_PWM = set->pwm_max_led;
	while(FCTL3 & BUSY);
	MEM_LED_MIN_PWM = set->pwm_min_led;
	while(FCTL3 & BUSY);
	MEM_FAN_MAX_PWM = set->pwm_max_fan;
	while(FCTL3 & BUSY);
	MEM_FAN_MIN_PWM = set->pwm_min_fan;
	while(FCTL3 & BUSY);
	MEM_FAN_OUT_OFF = set->fan_out_off;
	while(FCTL3 & BUSY);
	MEM_CYCLE_FORM = set->cycle_form;
	while(FCTL3 & BUSY);

		/* Simple check if everything was written */
	if(MEM_CYC +
			MEM_LED_MAX_PWM +
			MEM_FAN_MAX_PWM +
			MEM_LED_MIN_PWM +
			MEM_FAN_MIN_PWM +
			MEM_FAN_OUT_OFF +
			MEM_CYCLE_FORM ==
	set->cycle_time +
	set->pwm_max_led +
	set->pwm_max_fan +
	set->pwm_min_fan +
	set->pwm_min_led +
	set->fan_out_off +
	set->cycle_form){
		retVal = 0;
	}
	FCTL1 = FWKEY;					// Clear write bit
	FCTL3 = FWKEY + LOCK;			// set lock bit
	return retVal;
}

/* mem2Settings
 * Sets values from flash to Settings
 */
void mem2Settings(Settings *set){
	set->cycle_time = MEM_CYC;
	set->pwm_max_led = MEM_LED_MAX_PWM;
	set->pwm_max_fan = MEM_FAN_MAX_PWM;
	set->pwm_min_led = MEM_LED_MIN_PWM;
	set->pwm_min_fan = MEM_FAN_MIN_PWM;
	set->fan_out_off = MEM_FAN_OUT_OFF;
	set->cycle_form = MEM_CYCLE_FORM;
	setHelpers(set);
}

/* Calculate values used by timer 0 ISR */
void setHelpers(Settings *set) {
	set->pwm_step_led = (set->pwm_max_led - set->pwm_min_led)/ (set->cycle_time / 10);
	set->pwm_step_fan = (set->pwm_max_fan - set->pwm_min_fan)/ (set->cycle_time / 10);
	if(set->pwm_step_fan < 2 || set->pwm_step_led < 2) {
		uart_puts((char *)"Arvot voivat aiheuttaa virheita tai epatarkkuuksia!!!\r\n");
	}
	if(set->pwm_step_fan == 0) {
		uart_puts((char *)"Tuulettimen teho ei muutu!!!\r\n");
	} else if(set->pwm_step_led == 0) {
		uart_puts((char *)"Ledien teho ei muutu!!!\r\n");
	}

}

/* scaleValues
 * Scale time and pwm power values from input to usable
 */
uint16_t scaleValues(uint16_t value, int i) {
	switch(i)  {
		case 1:
			return TIMER0_SCALE_VAL * value;		// 0.1seconds to interrupts
													// max value for interrupt counter
		case 2:
			return value * PWM_SCALE_VAL;			// 0-100% power

		default:
			break;

	}
	return value;
}




