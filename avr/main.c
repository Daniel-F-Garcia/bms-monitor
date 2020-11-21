#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "display.h"
#include "usart.h"
#include "bms.h"

#define BUFF_SIZE   64

char * tohex(uint8_t * in, size_t size) {
	char * ret = malloc(size*3);
	
    uint8_t * pin = in;
    const char * hex = "0123456789ABCDEF";
    char * pout = ret;
    for(; pin < in+size; pout +=3, pin++){
        pout[0] = hex[(*pin>>4) & 0xF];
        pout[1] = hex[ *pin     & 0xF];
        pout[2] = ' ';
        if (pout + 3 - ret > size*3){
            break;
        }
    }
    pout[-1] = 0;
	
	return ret;
}


void bms_request_start() {
	uart_flush();
	uart_putstrl("\xDD\x5A\x00\x02\x56\x78\xFF\x30\x77", 9);
}

void bms_request_end() {
	uart_flush();
	uart_putstrl("\xDD\x5A\x01\x02\x00\x00\xFF\xFD\x77", 9);
}

struct BMSGeneral bms_request_general() {
	struct BMSGeneral ret;
	ret.valid = 0;
	
	//uart_flush();
	unsigned char data[] = {0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77};
	for (int i=0; i<7; i++) {
		uart_putc(data[i]);
	}

	uint8_t buffer[BUFF_SIZE];
	uint8_t response_length = bms_get_response(buffer);
	
	if (response_length==0) {
		display_set_text("t4", "EMPTY");
	} else {
		char * temp = tohex(buffer, response_length);
		display_set_text("t4", "%d: %s", response_length, temp);
	}
	
	if (response_length<4) {
		return ret;
	}
	
	if (buffer[0]!=0xDD || buffer[1]!=0x03 || buffer[2]!=0x00 || buffer[3]<0x19) {
		return ret;
	}
	
	ret.pack_voltage = (buffer[4] << 8) | buffer[5];
	ret.pack_current = (buffer[6] << 8) | buffer[7];
	if (ret.pack_current==0) {
		ret.state = 0;
	} else if (ret.pack_current>0) {
		ret.state = 1;
	} else {
		ret.state = -1;
	}
	ret.pack_current = abs(ret.pack_current);
	ret.residual_capacity = (buffer[8] << 8) | buffer[9];
	ret.nominal_capacity = (buffer[10] << 8) | buffer[11];
	if (ret.nominal_capacity==0) {
		ret.percent_capacity = -1;
	} else {
		ret.percent_capacity = (uint32_t)ret.residual_capacity * 100 / ret.nominal_capacity;
	}
	ret.temperature = ((buffer[27] << 8) | buffer[28]) - 2731;
	ret.valid = 1;
	
	return ret;
}

void bms_request_voltage() {
	char data[8] = {0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77};
	for (int i=0; i<7; i++) {
		uart_putc(data[i]);
	}
}

int main(void) {
	display_init();
	uart_set_FrameFormat(USART_8BIT_DATA|USART_1STOP_BIT|USART_NO_PARITY|USART_ASYNC_MODE);
	uart_init(BAUD_CALC(9600));
	
	stdout = &uart0_io;
	stdin = &uart0_io;
	sei();
	
	_delay_ms(100);
	bms_request_general();

	
	while(1) {
		//display_uart_puts("bar.val=70\xFF\xFF\xFF");
		
		display_set_text("t4", "wait...");
		_delay_ms(500);
		
		struct BMSGeneral bms_general = bms_request_general();
		
		
		if (bms_general.valid) {
			display_set_text("t0", "%d.%02d", bms_general.pack_voltage/100, bms_general.pack_voltage % 100);
			if (bms_general.state==-1) {
				display_set_text("t1", "-%d.%02d", bms_general.pack_current/100, bms_general.pack_current % 100);
			} else {
				display_set_text("t1", "%d.%02d", bms_general.pack_current/100, bms_general.pack_current % 100);
			}
			if (bms_general.percent_capacity==-1) {
				display_set_text("t2", "ERR");
			} else {
				display_set_text("t2", "%01d", bms_general.percent_capacity);
				display_set_bar("bar", bms_general.percent_capacity);
			}
			if (bms_general.temperature<0) {
				display_set_text("t3", "-%d.%01d", bms_general.temperature/10, bms_general.temperature % 10);
			} else {
				display_set_text("t3", "%d.%01d", bms_general.temperature/10, bms_general.temperature % 10);
			}
			
		} else {
			display_set_text("t0", "ERR");
			display_set_text("t1", "ERR");
		}
		

		_delay_ms(2500);
	}

    return 0;
}


