/*
 *uart.c
 *Version: 1.0
 *Parker Dillmann
 *The Longhorn Engineer (c) 2013
 *www.longhornengineer.com
 *
 *Check bottom of file for License.
 *
 *This is a uart_fifo.c file of the FIFO UART Hardware demo using USCI on the MSP430G2553 microcontroller.
 *Set your baud rate in your terminal to 9600 8N1.
 *When using your TI MSP-430 LaunchPad you will need to cross the TXD and RXD jumpers.
 *
 *
 */

/*
 *Small changes to original made to fit own purposes
 *
*/

#include <msp430g2553.h>
#include "uart.h"

#define RXD		BIT1
#define TXD		BIT2
#define FIFO_SIZE 128

volatile unsigned char tx_char;				// This char is the most current char to go into the UART

volatile unsigned int rx_flag;				// Mailbox Flag for the rx_char.
volatile unsigned char rx_char;				// This char is the most current char to come out of the UART

volatile unsigned char tx_fifo[FIFO_SIZE];	// The array for the tx fifo
volatile unsigned char rx_fifo[FIFO_SIZE];  // The array for the rx fifo

volatile unsigned int tx_fifo_ptA;			// Theses indexes keep track where the UART and the Main program are in the Fifos
volatile unsigned int tx_fifo_ptB;			// A is oldest, B is in front of latest
volatile unsigned int rx_fifo_ptA;			// Read A, write to B
volatile unsigned int rx_fifo_ptB;

volatile unsigned int rx_fifo_full;
volatile unsigned int tx_fifo_full;

/* Setup for uart connection */
void uart_init() {
	P1SEL |= RXD + TXD;					// Set pins for UART
	P1SEL2 |= RXD + TXD;

	UCA0CTL1 |= UCSSEL_2; 				// use SMCLK
	UCA0BR0 = 52;                  		// 8MHz, OSC16, 9600, Set baud rate
	UCA0BR1 = 0;                   	 	// ((8MHz/9600)/16) = 52.08333
	UCA0MCTL = 0x10|UCOS16; 			// UCBRFx=1,UCBRSx=0, UCOS16=1
	UCA0CTL1 &= ~UCSWRST; 				// USCI state machine
	IE2 |= UCA0RXIE;					// Enable interrupt

	rx_flag = 0;						//Set rx_flag to 0

	tx_fifo_ptA = 0;					//Set the fifo pointers to 0
	tx_fifo_ptB = 0;
	rx_fifo_ptA = 0;
	rx_fifo_ptB = 0;

	tx_fifo_full = 0;
	rx_fifo_full = 0;

}

/* uart_getc
 * Get char from the UART
 * RETURN: char from UART
 */
unsigned char uart_getc() {
	unsigned char c;

	while(rx_flag == 0); 				// Wait until char exists to get


	c = rx_fifo[rx_fifo_ptA];			// Get char from fifo buffer
	rx_fifo_ptA++;

	if(rx_fifo_ptA == FIFO_SIZE) {		// Roll over if at the end of fifo buffer
		rx_fifo_ptA = 0;
	}

	if(rx_fifo_ptA == rx_fifo_ptB) {	// No new data if indexes are same
		rx_flag = 0;
	}

	return c;
}

/* uart_gets
 * Get a string of known length from the UART
 * String is terminated before length is read on carriage return
 * INPUT: Array pointer and length
 * RETURN: None
 */
void uart_gets(char* array, int length) {
	unsigned int i = 0;					// String index counter

	while(i < length) {					// Get data until specified length
		array[i] = uart_getc();

		if(array[i] == '\r') {			// Fill rest of lenght with \0
			array[i] = uart_getc();		// Since terminal adds \r\n read last char and overwrite it with \0
			for( ; i < length; i++) {
				array[i] = '\0';
			}
			break;
		}
		i++;
	}
}

/* uart_putc
 * Sends a char to UART, will wait for UART if busy
 * INPUT: Char to send
 * RETURN: None
 */
void uart_putc(unsigned char c) {
	tx_char = c;
	tx_fifo[tx_fifo_ptA] = tx_char;		// Put char into tx fifo
	tx_fifo_ptA++;

	if(tx_fifo_ptA == FIFO_SIZE) {		// Roll over if at the end of buffer
		tx_fifo_ptA = 0;
	}
	if(tx_fifo_ptB == tx_fifo_ptA) {	// fifo full
		tx_fifo_full = 1;
	}
	else {
		tx_fifo_full = 0;
	}
	IE2 |= UCA0TXIE; 					//Enable USCI_A0 TX interrupt

}



/* uart_puts
 * Sends string to UART
 * INPUT: Pointer to string
 * RETURN: None
*/
void uart_puts(char *str) {
	while(*str) {
		uart_putc(*str++);				// Call putc until string end
	}
	return;
}


/* INTERRUPT SERVICE ROUTINES */

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {

	UCA0TXBUF = tx_fifo[tx_fifo_ptB];		// Set char from fifo to TX buffer
	tx_fifo_ptB++;

	if(tx_fifo_ptB == FIFO_SIZE) {			// Roll over if fifo is full
		tx_fifo_ptB = 0;
	}
	if(tx_fifo_ptB == tx_fifo_ptA) {		// No data to transmit so turn off interrupt
		IE2 &= ~UCA0TXIE;
	}
}


#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	rx_char = UCA0RXBUF;					// Copy char from RX buffer
	rx_flag = 1;							// Set received flag

	rx_fifo[rx_fifo_ptB] = rx_char;			// Copy rx char to fifo
	rx_fifo_ptB++;

	if(rx_fifo_ptB == FIFO_SIZE) {			// Roll over if ptB at end of fifo
		rx_fifo_ptB = 0;
	}
	if(rx_fifo_ptB == rx_fifo_ptA) {		// Fifo full
		rx_fifo_full = 1;
	}
	else {
		rx_fifo_full = 0;
	}

}




/*
â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”�
â”‚                           TERMS OF USE: MIT License                                  â”‚
â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¤
â”‚Permission is hereby granted, free of charge, to any person obtaining a copy of this  â”‚
â”‚software and associated documentation files (the "Software"), to deal in the Software â”‚
â”‚without restriction, including without limitation the rights to use, copy, modify,    â”‚
â”‚merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    â”‚
â”‚permit persons to whom the Software is furnished to do so, subject to the following   â”‚
â”‚conditions:                                                                           â”‚
â”‚                                                                                      â”‚
â”‚The above copyright notice and this permission notice shall be included in all copies â”‚
â”‚or substantial portions of the Software.                                              â”‚
â”‚                                                                                      â”‚
â”‚THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   â”‚
â”‚INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         â”‚
â”‚PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    â”‚
â”‚HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION     â”‚
â”‚OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE        â”‚
â”‚SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                â”‚
â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
*/

