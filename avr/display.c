#include <stdlib.h>
#include <stdarg.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "display.h"

// Based on https://github.com/MarcelMG/AVR8_BitBang_UART_TX

volatile uint16_t tx_shift_reg = 0;

void display_init() {
   //set TX pin as output
   DISPLAY_TX_DDR |= _BV(DISPLAY_TX_DDR_PIN);
   DISPLAY_TX_PORT |= _BV(DISPLAY_TX_PIN);
   //set timer0 to CTC mode
   TCCR0A = _BV(WGM01);
   //enable output compare 0 A interrupt
   TIMSK0 = _BV(OCIE0A);
   //set compare value to 103 to achieve a 9600 baud rate (i.e. 104µs)
   //together with the 8MHz/8=1MHz timer0 clock
   /*NOTE: since the internal 8MHz oscillator is not very accurate, this value can be tuned
     to achieve the desired baud rate, so if it doesn't work with the nominal value (103), try
     increasing or decreasing the value by 1 or 2 */
   OCR0A = 103;
}

void display_uart_put(char character) {
   uint16_t local_tx_shift_reg = tx_shift_reg;
   //if sending the previous character is not yet finished, return
   //transmission is finished when tx_shift_reg == 0
   if(local_tx_shift_reg){return;}
   //fill the TX shift register witch the character to be sent and the start & stop bits (start bit (1<<0) is already 0)
   local_tx_shift_reg = (character<<1) | (1<<9); //stop bit (1<<9)
   tx_shift_reg = local_tx_shift_reg;
   // clock / 8
   TCCR0B = _BV(CS01);
}

void display_uart_puts(char* string) {
    while( *string ){
        display_uart_put( *string++ );
        //wait until transmission is finished
        while(tx_shift_reg);
    }
}

void display_set_text(char* variable, char *format, ...) {
	va_list valist;
	
	char value[50];
	sprintf(value, format, valist);
	
	display_uart_puts(variable);
	display_uart_puts(".txt=\"");
	//uart_bb_tx_str(value);
	display_uart_puts(format);
	display_uart_puts("\"\xFF\xFF\xFF");
}

//timer0 compare A match interrupt
ISR(TIMER0_COMPA_vect) {
   uint16_t local_tx_shift_reg = tx_shift_reg;
   //output LSB of the TX shift register at the TX pin
   if( local_tx_shift_reg & 0x01 ) {
      DISPLAY_TX_PORT |= _BV(DISPLAY_TX_PIN);
   } else {
      DISPLAY_TX_PORT &=~ _BV(DISPLAY_TX_PIN);
   }
   //shift the TX shift register one bit to the right
   local_tx_shift_reg >>= 1;
   tx_shift_reg = local_tx_shift_reg;
   //if the stop bit has been sent, the shift register will be 0
   //and the transmission is completed, so we can stop & reset timer0
   if(!local_tx_shift_reg)
   {
      TCCR0B = 0;
      TCNT0 = 0;
   }
}