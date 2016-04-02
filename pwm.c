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
#include "pwm.h"
#include "main.h"

#define PWM1 BIT5	// Pin definitions for pwm signals
#define PWM2 BIT2



/* TA_init
 * Initialize A timers 0 and 1 and define pins for pwm signals
 * Set timer roll over point and init values for pwm power compare points
 * INPUT: None
 */
void TA_init(void) {

	/*Set pwm out pins as timer outs*/
	P2DIR |= PWM1 + PWM2;
	P2SEL |= PWM1 + PWM2;

	/*Timer 0 cycle init*/
	TA0CCR0 = TIMER0_MAX_COUNT;
	TA0CCTL0 |= CCIE;
	TA0CTL |= TASSEL_2 + ID_3 + MC_1;  // SMCLK, div 8, Up Mode , interrupt enable


	/*Timer 1 pwm init*/
	TA1CCR0 = TIMER1_MAX_COUNT;
	TA1CCR1 = 0;
	TA1CCR2 = 0;

	TA1CTL |= TASSEL_2 + ID_0 + MC_1;  // SMCLK, div 0, Up Mode

	TA1CCTL1 |= OUTMOD_7;		// Reset at TACCR1
	TA1CCTL2 |= OUTMOD_7;		// Reset at TACCR2


}


/* set_pwm_dc
 * Sets duty cycle to pwm x
 * INPUT: RunValues which to set to CCR registers
 * RETURN: None
 */
void set_pwm_dc(RunValues *rv) {
	TA1CCR1 = rv->pwm_dc_led;
	TA1CCR2 = rv->pwm_dc_fan;
}

/* reset_run_values
 * Resets all values to 0 except dir = 1
 * Call after changing settings to reset cycle
 * INPUT: RunValues to reset
 */
void reset_run_values(RunValues *rv) {
	rv->inter_cycles = 0;
	rv->pwm_dc_fan = 0;
	rv->pwm_dc_led = 0;
	rv->help_count = 0;
	rv->dir = 1;
}

/* pwm_cycle_isrf
 * Place inside ISR in main using TIMER0_A0_VECTOR or other timer vector
 * INPUT: RunValues and Settings for cycle and power control
 */

void pwm_cycle_isrf(RunValues *rv, Settings *set) {
	/* IF full cycle is done */
	if(rv->inter_cycles >= set->cycle_time) {
		/* Change direction when interrupt counter at cycle time and set max or min as pwm power*/
		if(rv->dir == 1) {
			rv->pwm_dc_led = set->pwm_max_led;
			rv->pwm_dc_fan = set->pwm_max_fan;
			set_pwm_dc(rv);						// Set max values on run values and change direction
			rv->dir = -1;
		} else {
			rv->pwm_dc_led = set->pwm_min_led;
			rv->pwm_dc_fan = set->pwm_min_fan;					// Set run values to min and change direction
			set_pwm_dc(rv);
			rv->dir = 1;
		}
		/* Reset counters */
		rv->inter_cycles = 0;
		rv->help_count = 0;
	/* Every second interrupt change pwm power by step toward dir*/
	} else if(rv->help_count >= 1) {

		rv->pwm_dc_fan += rv->dir * set->pwm_step_fan;
		rv->pwm_dc_led += rv->dir * set->pwm_step_led;
		set_pwm_dc(rv);
		rv->help_count = 0;
		rv->inter_cycles++;
	} else {
		rv->help_count++;
		rv->inter_cycles++;		// if no roll over count up
	}


}







