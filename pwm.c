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
 *      		This uses pins 2.5 and 2.2 for PWM signals, 2.5 for fan 2.2 for led
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
#include <math.h>  // sin or pow eats all mem defined for .stack and .const
#include "settings.h"
#include "pwm.h"
#include "main.h"


#define PWM2 BIT5	// Pin definitions for pwm signals
#define PWM1 BIT2
#define PI 3.14159265

volatile double val_sin = 0;

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
	TA0CCTL0 |= CCIE;			// Enable interrupt on CCR0
	TA0CTL |= TASSEL_2 + ID_1 + MC_1;  // SMCLK, div 2, Up Mode , interrupt enable


	/*Timer 1 pwm init*/
	TA1CCR0 = TIMER1_MAX_COUNT;
	TA1CCR1 = 0;
	TA1CCR2 = 0;

	TA1CTL |= TASSEL_2 + ID_0 + MC_1;  // SMCLK, div 1, Up Mode

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

/* pwm_triangle_cycle_isrf
 * Place inside ISR in main using TIMER0_A0_VECTOR or other timer vector
 * INPUT: RunValues and Settings for cycle and power control
 */

void pwm_triangle_cycle_isrf(RunValues *rv, Settings *set) {		// IF SCALING MAX VALUE WORKS COMBINE AND MAKE FUNCTION FOR SCALE VAL CALC
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
	} else if(rv->help_count >= INT_DELAY) {

		if(set->fan_out_off == 1 && rv->dir == -1) {	// Keep fan at 0 when breathing out
			rv->pwm_dc_fan = set->pwm_min_fan;
		} else if(set->fan_out_off == 1 && rv->dir == 1) {		// fan at max when breathing in
			rv->pwm_dc_fan = set->pwm_max_fan;
		} else {
			rv->pwm_dc_fan += rv->dir * set->pwm_step_fan;		// if fan set to follow leds
		}

		rv->pwm_dc_led += rv->dir * set->pwm_step_led;			// Leds at triangle wave
		set_pwm_dc(rv);
		rv->help_count = 0;
		rv->inter_cycles++;

	} else {
		rv->help_count++;
		rv->inter_cycles++;		// if no roll over count up
	}
}

/* pwm_sin_cycle_isrf
 *
 *
 */
/*void pwm_sin_cycle_isrf(RunValues *rv, Settings *set) {


}*/

double get_sin2_appr(double x) {
	if(x < 0 && x > PI*0.5) {
		x -= (x - PI*0.5);		// Mirror values after PI/2 because approximation at y(PI) != 0
	}
	return get_pow((0.9999966 * x - 0.16664824 * get_pow(x, 3) + 0.00830629 * get_pow(x, 2) - 0.00018363 * get_pow(x, 2)), 2);
	//return get_pow(0.5, 2);
}

/* Calculate pow if i for x
 * power must be integer
 * INPUT: double x, integer i
 * RETURN: double x^i
 */
double get_pow(double x, int i) {
	double y = 1;
	int n;
	for(n = 0; n < i; n++) {
		y *= x;
	}
	return y;
}

/* pwm_on_off_cycle_isrf
 *
 *
 */
void pwm_sin_cycle_isrf(RunValues *rv, Settings *set) {
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
	} else if(rv->help_count >= INT_DELAY) {

		if(rv->dir == -1) {
			if(set->fan_out_off == 1) {
				rv->pwm_dc_fan = set->pwm_min_fan;
			}
			val_sin = 1.0867 * get_sin2_appr(PI*0.5 - (((float)rv->inter_cycles/(float)set->cycle_time)*PI*0.45615));
			// 1.0867 == siniamplitudin korjauskerroin                    == PI*0.5*0.9123 (sinix korjauskerroin)

		} else if(rv->dir == 1) {
			if(set->fan_out_off == 1) {
				rv->pwm_dc_fan = set->pwm_max_fan;
			}
			val_sin = 1.0867 * get_sin2_appr(((float)rv->inter_cycles/(float)set->cycle_time)*PI*0.45615);
		}
		if(set->fan_out_off == 0) {
			rv->pwm_dc_fan = (uint16_t) (val_sin * (set->pwm_max_fan - set->pwm_min_fan) + set->pwm_min_fan);
		}


		rv->pwm_dc_led = (uint16_t) (val_sin * (set->pwm_max_led - set->pwm_min_led) + set->pwm_min_led);
		// Max led value scaled with sin^2 value

		set_pwm_dc(rv);
		rv->help_count = 0;
		rv->inter_cycles++;

	} else {
		rv->help_count++;
		rv->inter_cycles++;		// if no roll over count up
	}
}



