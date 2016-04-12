/*
 * pwm.h
 *
 */

#ifndef PWM_H_
#define PWM_H_

#include "main.h"

#define TIMER1_MAX_COUNT 32000		// PWM frequency - DCO/TIMER_MAX_COUNT - 8Mhz/(4 * 8000) = 250Hz
#define PWM_SCALE_VAL TIMER1_MAX_COUNT/100
#define TIMER0_MAX_COUNT 40000
#define TIMER0_FRQ 4000000			// DCO / div
#define TIMER0_SCALE_VAL TIMER0_FRQ/TIMER0_MAX_COUNT/10		// interrupts/0.1s = 100

/*	Values to use for cycling pwm power, used as current values
 *
 *
 *
 */
/*
typedef struct run_values {
	volatile uint16_t inter_cycles;
	volatile uint16_t pwm_dc_led;
	volatile uint16_t pwm_dc_fan;
	volatile int dir;
	volatile int help_count;

} RunValues;
*/

/* TA_init
 * Initialize A timers 0 , 1 and
 *
 */
void TA_init(void);

void set_pwm_dc(RunValues *rv);

void reset_run_values(RunValues *rv);

void pwm_triangle_cycle_isrf(RunValues *rv,  Settings *set);

void pwm_sin_cycle_isrf(RunValues *rv, Settings *set);

uint16_t cycletime2counter(uint16_t value);


#endif /* PWM_H_ */
