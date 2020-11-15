#include <avr/io.h>
#include <stdlib.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

#include "usart.h"

#define BUFF_SIZE   25

int main(void)
{
	uart_init(BAUD_CALC(9600));   //  Nextion
	
	stdout = &uart0_io;
	stdin = &uart0_io;
	sei();
	
	
	while(1) {
		uart_puts("t0.txt=\"13.0\"\xFF\xFF\xFF");
		uart_puts("bar.val=66\xFF\xFF\xFF");
		_delay_ms(500);
	}

    return 0;
}
