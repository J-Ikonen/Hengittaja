/*
 * pwm.c
 *
 *  Created on: 31.3.2016
 *      Author: Joonas
 *
 *      Pins   	1.5 Timer0_A compare Out0	TA0.0
 *      		1.6 Timer0_A compare Out1	TA0.1
 *      		2.0 Timer1_A compare Out0	TA1.0	Timer1_A3.TA0, P2SEL = 1, P2SEL2 = 0
 *      		2.1 /2.2					TA1.1	Timer1_A3.TA1 	P2DIR = 1, P2SEL = 1, P2SEL2 = 0
 *      		2.3 / 2.4					TA1.2
 *
 *      		Timer_A up/down mode - MCx == 11, Output mode 6 seems good, 2 ok
 *      		CAP = 0, compare mode, CCIFG is set when TAR counts to TACCRx, (also EQUx = 1, )
 *
 *      		Interrupt vectors
 *      		TACCR0 for TACCR0 CCIFG
 *      		TAIV for all other CCIFG flags and TAIFG
 *
 */


#include <msp430g2553.h>

#define PWMFRQ 8000		// PWM frequency - DC0/PWMFRQ - 8Mhz/1000 = 8kHz


void TA_init(void) {
	TACCR0 = PWMFRQ;
	TACCR1 = PWMFRQ/2;
	//TACCR2 = PWMFRQ/2;

	TACTL = TASSEL_2 + ID_0 + MC_1;  // SMCLK, div 1, Up Mode  (+ TAIE)

	TACCTL1 = OUTMOD_7;		// Reset at TACCR1
	//TACCTL2 = OUTMOD_7;		// Reset at TACCR2

}

/* set_pwm_dc
 * Sets duty cycle to pwm x
 * INPUT: Duty cycle dc, i pwm 1 or 2
 * RETURN: None
 */
void set_pwm_dc(int dc, int i) {
	if(i == 1) {
		TACCR1 = (PWMFRQ / 100) * dc;
	} else if(i == 2) {
		TACCR2 = (PWMFRQ / 100) * dc;
	} else {

	}
}
