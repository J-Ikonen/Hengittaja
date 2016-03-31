#include <msp430g2553.h>
#include <inttypes.h>
#include "settings.h"
#include <string.h>
#include "uart.h"
#include "pwm.h"

/*
 * main.c
 */
 
/* Readable pin defines and other constants */
#define RLED 	BIT0
#define GLED	BIT6
#define RX		BIT1
#define TX		BIT2
#define INPUT_SIZE 30

/* FUNCTION DECLARATIONS */
void board_setup(void);
void delay_cycles(volatile uint32_t i);


/* GLOBAL VARIABLES */
char rx_string[INPUT_SIZE + 1];


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL1 = CALBC1_8MHZ; 				//Set DCO to 8Mhz
    DCOCTL = CALDCO_8MHZ; 				//Set DCO to 8Mhz

    board_setup();
    uart_init();
    TA_init();
    __enable_interrupt();
	
    Settings set;
	mem2Settings(&set);

	
    while(1) {
		if(rx_flag == 1) {
    		uart_putc(uart_getc());			// getc fixes one line lag on reading commands somehow
    		memset(rx_string, 0, INPUT_SIZE);		// getting empty line on console means faulty command
    		uart_gets(rx_string, INPUT_SIZE);		// se on ominaisuus ei bugi
			/*We have the data! 
			Format should be the following: 1:120&2:100&3:80&5:00
											^ ^
											| |																
											| Value: for cycletimes: 0-65 (0.1s) eg. 10s = 100
											|		 for pwm: 0-99 (%) , when writing value doesnt do anything
											|
											What to do: 1: change led cycletime, 2: change fan cycletime
											3: change led pwm brightness, 4: fan pwm power
											5: write current settings to flash memory
																
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
					++separator;
					int val = atoi(separator);
					
					changeSettings(set, memtask, val); // Call changeSettings to change settings or write them2flash
					
				}
				// Find the next command in input string
				command = strtok(0, "&");
			}
    	}
		//____MISSING WHAT TO DO WITH SETTINGS
    }
	
	return 0;
}

/* Pin and other setup at start _____NOT REVISED____ */
void board_setup(void) {

	P1OUT = 0;
	P1DIR |= RLED + GLED;
	P1SEL |= GLED;		// Set GLED as TA0.1


}

/* Delay i cycles */
void delay_cycles(volatile uint32_t i) {
	do i--;
	while(i != 0);
}
