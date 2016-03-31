#include <msp430g2553.h>
#include <inttypes.h>
#include "settings.h"


/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
    Settings set;

	set.cycle_time_led = 10000;
	set.cycle_time_fan = 20000;
	set.pwm_max_led = 80;
	set.pwm_max_fan = 60;

	settings2Mem(&set);

	set.cycle_time_led = 50000;
	set.cycle_time_fan = 5000;
	set.pwm_max_led = 5;
	set.pwm_max_fan = 10;

	mem2Settings(&set);


    while(1) {


    }


	return 0;
}
