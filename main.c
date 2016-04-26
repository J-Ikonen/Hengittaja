#include <msp430g2553.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include "settings.h"
#include "uart.h"
#include "pwm.h"
#include "main.h"

/*
 * main.c
 * Käyttöohje  v0.1
 *
 * Käyetetyt pinnit:
 * P1.1 == RX   ---> TX HC-06
 * P1.2 == TX   ---> RX HC-06
 * P2.2 == PWM1 ---> LED
 * P2.5 == PWM2 ---> FAN
 *
 *
 * Korkein aika jolla pwm teho vielä vaihtuu saadaan yhtälöllä
 * (max - min) * 80 / 10 = MaxAika,
 * missä max ja min (%), MaxAika (s)
 */
 
/* Readable pin defines and other constants */
#define RLED 	BIT0
#define GLED	BIT6
#define RX		BIT1
#define TX		BIT2
#define INPUT_SIZE 30



/* GLOBAL VARIABLES */

RunValues rVal;
Settings set;

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;			// Stop watchdog timer
    BCSCTL1 = CALBC1_16MHZ; 			//Set DCO to 8Mhz
    DCOCTL = CALDCO_16MHZ; 				//Set DCO to 8Mhz
    __delay_cycles(50000);				// Delay to let the clock set to cal value


    board_setup();
    uart_init();
    reset_run_values(&rVal);

    if(mem2Settings(&set) != 1) {
    	 settingsDefault(&set);
    }
    							// Set values used by timer before timer init
    							// CHANGE TO LOAD FROM MEMORY AND IF SHITTY MEMORY THEN DEFAULTS
    TA_init();
    __enable_interrupt();

    printHelp();
	
    // Init settings and run values


	
    while(1) {
    	if(rx_flag == 1) {
			get_bt_data();
		}
    }
	
	return 0;
}

/* Pin and other setup at start _____NOT REVISED____ */
void board_setup(void) {
	P1OUT = 0;
	P1DIR |= RLED + GLED;
}

/* Delay i cycles */
void delay_cycles(volatile uint32_t i) {
	do i--;
	while(i != 0);
}

void get_bt_data(void) {
	char rx_string[INPUT_SIZE];
	memset(rx_string, 0, INPUT_SIZE);
	uart_gets(rx_string, INPUT_SIZE);
	/*We have the data!
	Format should be the following: 1:120&2:100&3:80&5:00
									^ ^
									| |
									| Value: for cycletimes: 0-65 (0.1s) eg. 10s = 100
									|		 for pwm: 0-99 (%) , when writing value doesnt do anything
									|
									What to do: 1: change cycletime, 2: change led pwm max
									3: change led pwm min, 4: fan pwm max
									5: fan pwm min
									9: set default settings
									10: write current settings to flash memory
									11: get settings from memory (discard current settings)

									where is enum keyword structure for commands...

	Let's start parsing away!							*/

	/* Following code for using data strings without leaving memory full*/
	char* command = strtok(rx_string, "&");
	while (command != 0)
	{
		// Split the command in two values
		char* separator = strchr(command, ':');
		if (separator != 0)
		{
			// Actually split the string in 2: replace ':' with 0
			*separator = 0;


			int memtask = atoi(command);
			if(memtask == 0) {		// if not proper command print help
				printHelp();
				break;
			}
			++separator;
			int val = atoi(separator);

			changeSettings(&set, memtask, val, &rVal); // Call changeSettings to change settings or write them2flash

		} else {
			uart_puts((char *)"Kaskyn loputtava ':' merkkiin.\n");
		}
		// Find the next command in input string
		command = strtok(0, "&");
	}

}

/* INTERRUPT SERVICE ROUTINES 	*/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void) {
	if(rx_flag == 1) {
		get_bt_data();
	} else
		switch(set.cycle_form) {
			case 1:
				pwm_sin_cycle_isrf(&rVal, &set);
				break;
			case 2:

				pwm_triangle_cycle_isrf(&rVal, &set);

				break;
			case 3:

				break;
			default:
				break;
		}


}










