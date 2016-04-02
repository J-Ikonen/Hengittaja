/*
 * pwm.c
 *
 *  Created on: 31.3.2016
 *      Author: Joonas Ikonen
 *
 *		Pins for msp430g2553
 *      Pins   	1.5 Timer0_A compare Out0	TA0.0
 *      		1.6 Timer0_A compare Out1	TA0.1
 *      		3.0 / 3.6					TA0.2 ????	Does not exist on 20pin devices
 *      		2.0 Timer1_A compare Out0	TA1.0	Timer1_A3.TA0, P2SEL = 1, P2SEL2 = 0
 *      		2.1 /2.2					TA1.1	Timer1_A3.TA1 	P2DIR = 1, P2SEL = 1, P2SEL2 = 0
 *      		2.3 / 2.4					TA1.2
 *
 *      		This uses pins 2.5 and 2.2 for PWM signals
 *
 *				Need to use Timer1_A for 2 differing pwm signals, and Timer0_A for timing cycles
 *
 *   			Up mode with OUTMOD_7 for 2 different pwm signals with free ccr0 to choose roll over point
 *      		CCIFG is set when TAR counts to TACCRx, (also EQUx = 1, )
 *
 *      		Interrupt vectors
 *      		TACCR0 for TACCR0 CCIFG
 *      		TAIV for all other CCIFG flags and TAIFG
 *
 *      		Up mode TAIFG flag is set when timer counts from TACCR0 to zero
 *
 */


#include <msp430g2553.h>
#include <inttypes.h>
#include "settings.h"

#define PWM1 BIT5	// Pin definitions for pwm signals
#define PWM2 BIT2



/* TA_init
 * Initialize A timers 0 and 1 and define pins for pwm signals
 * Set timer roll over point and init values for pwm power compare points
 *
 */
void TA_init(RunValues *rv) {
	/*Set pwm out pins as timer outs*/
	P2DIR |= PWM1 + PWM2;
	P2SEL |= PWM1 + PWM2;

	/*Timer 0 init*/
	TA0CCR0 = TIMER0_MAX_COUNT;
	TA0CTL |= TASSEL_2 + ID_0 + MC_1 + TAIE;  // SMCLK, div 8, Up Mode , interrupt enable


	/*Timer 1 init*/
	TA1CCR0 = TIMER_MAX_COUNT;
	TA1CCR1 = rv->pwm_dc_led;
	TA1CCR2 = rv->pwm_dc_fan;

	TA1CTL |= TASSEL_2 + ID_3 + MC_1;  // SMCLK, div 8, Up Mode

	TA1CCTL1 |= OUTMOD_7;		// Reset at TACCR1
	TA1CCTL2 |= OUTMOD_7;		// Reset at TACCR2
}


/* Init run values */
void init_rv(RunValues *rv, Settings *set) {
	rv->inter_cycles = set->cycle_time_led;
	rv->pwm_dc_fan = set->pwm_max_fan;
	rv->pwm_dc_led = set->pwm_max_led;
}

/* set_pwm_dc
 * Sets duty cycle to pwm x
 * INPUT: Duty cycle dc, set pwm choice ( 1 for led, 2 for fan, 3 for both)
 * RETURN: None
 */
void set_pwm_dc(RunValues *rv, int i) {
	switch(i) {
		case 1:
			TA1CCR1 = rv->pwm_dc_led;
			break;
		case 2:
			TA1CCR2 = rv->pwm_dc_fan;
			break;
		case 3:
			TA1CCR1 = rv->pwm_dc_led;
			TA1CCR2 = rv->pwm_dc_fan;
			break;
		default:
			break;
	}
}


void reset_run_values(RunValues *rv) {
	rv->inter_cycles = 200;
	rv->pwm_dc_fan = 0;
	rv->pwm_dc_led = 0;
	rv->help_count = 0;
}


void pwm_cycle_isrf(RunValues *rv, Settings *set) {
	/* IF full cycle is done */
	if(rv->inter_cycles == set->cycle_time_fan) {
		/* Change direction when interrupt counter at cycle time and set max or min as pwm power*/
		if(rv->dir == 1) {
			rv->pwm_dc_led = set->pwm_max_led;
			rv->dir = -1;
		} else {
			rv->pwm_dc_led = 0;
			rv->dir = 1;
		}
		/* Reset counters */
		rv->inter_cycles = 0;
		rv->help_count = 0;
	/* Every second interrupt change pwm power by step toward dir*/
	} else if(rv->help_count == 1) {
		rv->pwm_dc_led += rv->dir * set->pwm_step;
		rv->pwm_dc_fan += rv->dir * set->pwm_step;
		rv->help_count = 0;
	}

	rv->help_count++;
	rv->inter_cycles++;
}






