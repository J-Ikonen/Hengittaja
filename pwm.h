/*
 * pwm.h
 *
 *  Created on: 31.3.2016
 *      Author: Joonas
 */

#ifndef PWM_H_
#define PWM_H_

#define TIMER1_MAX_COUNT 8000		// PWM frequency - DCO/TIMER_MAX_COUNT - 8Mhz/8000 = 1kHz
#define TIMER0_MAX_COUNT 50000
#define TIMER0_FRQ 1000000			// DCO / div

/*	Values to use for cycling pwm power, used as current values
 *
 *
 *
 */
typedef struct run_values {
	volatile uint16_t inter_cycles;
	volatile uint16_t pwm_dc_led;
	volatile uint16_t pwm_dc_fan;
	volatile int dir;
	volatile int help_count;

} RunValues;


/* TA_init
 * Initialize A timers 0 , 1 and
 *
 */
void TA_init(RunValues *rv, Settings *set);
void set_pwm_dc(RunValues *rv, int i);
void reset_run_values(RunValues *rv);
void pwm_cycle_isrf(RunValues *rv,  Settings *set);
uint16_t cycletime2counter(uint16_t value);
void init_rv(RunValues *rv, Settings *set);

#endif /* PWM_H_ */
