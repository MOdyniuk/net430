/*
 * This file is part of the MSP430 hardware UART example.
 *
 * Copyright (C) 2012 Stefan Wendler <sw@kaltpost.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/******************************************************************************
 * Hardware UART example for MSP430.
 *
 * Stefan Wendler
 * sw@kaltpost.de
 * http://gpio.kaltpost.de
 ******************************************************************************/

#include <msp430.h>
#include <legacymsp430.h>
#include <stdlib.h>
#include "uart.h"
#include "config.h"

/**
 * Receive Data (RXD) at P1.1
 */
#define RXD		BIT1

/**
 * Transmit Data (TXD) at P1.2
 */
#define TXD		BIT2

#ifdef UART_ENABLE
/**
 * Callback handler for receive
 */
void (*uart_rx_isr_ptr)(unsigned char c);

void uart_init(void)
{
	uart_set_rx_isr_ptr(0L);

	P1SEL  |= RXD + TXD;
  	P1SEL2 |= RXD + TXD;
  	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
#if 0
  	UCA0BR0 = 0x41;         // 8MHz 9600
  	UCA0BR1 = 0x03;           // 8MHz 9600
#endif
  	UCA0BR0 = (FCPU/9600) & 0xFF;
  	UCA0BR1 = ((FCPU/9600) >> 8) & 0xFF;
  	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
  	UCA0CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
  	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

void uart_set_rx_isr_ptr(void (*isr_ptr)(unsigned char c))
{
	uart_rx_isr_ptr = isr_ptr;
}

unsigned char uart_getc()
{
    while (!(IFG2&UCA0RXIFG));                // USCI_A0 RX buffer ready?
	return UCA0RXBUF;
}

void uart_putc(unsigned char c)
{
	while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  	UCA0TXBUF = c;                    		// TX
}

void uart_puts(const char *str)
{
     while(*str) uart_putc(*str++);
}

#ifdef UART_LOG
void debug_puts(const char *str)
{
     while(*str) uart_putc(*str++);
}

void debug_puthex(uint16_t v) {
	char buf[10];
	itoa(v, buf, 16);
	debug_puts(buf);
}

void debug_nl() {
	debug_puts("\n");
}
#endif
interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
{
	__bic_SR_register_on_exit(CPUOFF);
	if(uart_rx_isr_ptr != 0L) {
		(uart_rx_isr_ptr)(UCA0RXBUF);
	}
}
#endif
