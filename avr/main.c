#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "display.h"
#include "usart.h"

#define BUFF_SIZE   64
#define BMS_RESPONSE_TIMEOUT 100
#define BMS_READ_TIMEOUT 2

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

/*char * tohexc(char * in, size_t size) {
	char * ret = malloc(size*3);
	
    char * pin = in;
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
}*/



uint8_t bms_get_response_byte(uint8_t * buffer, uint8_t timeout) {
	uint8_t status = BUFFER_EMPTY;
	
	uint8_t count = 0;
	while (1) {
		if (count>timeout) {
			break;
		}
		status = uart0_LoadData(buffer);
		if (status==COMPLETED) {
			break;
		}
		_delay_ms(1);
		count++;
	}
		
	return status;
}

uint8_t bms_get_response(uint8_t * buffer) {
	uint8_t * current = buffer;
	uint8_t response_length = 0;
	uint8_t status = bms_get_response_byte(current, BMS_RESPONSE_TIMEOUT);
	
	//if (status==COMPLETED) {
	//	_delay_ms(BMS_WAIT_AFTER_FIRST);
	//}
	
	while (status==COMPLETED) {
		current++;
		response_length++;
		status = bms_get_response_byte(current, BMS_READ_TIMEOUT);
	}
	
	return response_length;
}

void bms_request_start() {
	uart_flush();
	uart_putstrl("\xDD\x5A\x00\x02\x56\x78\xFF\x30\x77", 9);
}

void bms_request_end() {
	uart_flush();
	uart_putstrl("\xDD\x5A\x01\x02\x00\x00\xFF\xFD\x77", 9);
}

struct BMSGeneral {
	uint8_t valid;
	int voltage;
};

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
		//set_display_text("t4", "ok");
		display_set_text("t4", temp);
		//char temp[50];
		//sprintf(temp, "%02X:%02X:%02X:%02X:%02X:%02X %d", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], response_length);
		//set_display_text("t4", temp);
	}
	
	if (response_length<4) {
		return ret;
	}
	
	if (buffer[0]!=0xDD) {
		return ret;
	}
	
	ret.valid = 1;
	return ret;
}

void bms_request_voltage() {
	//uart_flush();
	char data[8] = {0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77};
	for (int i=0; i<7; i++) {
		uart_putc(data[i]);
	}
}

int main(void) {
	// 1 = output, 0 = input
	DDRD  = 0b00000010;
	PORTD = 0b00000001;
	
	display_init();
	
	uart_set_FrameFormat(USART_8BIT_DATA|USART_1STOP_BIT|USART_NO_PARITY|USART_ASYNC_MODE);
	uart_init(BAUD_CALC(9600));   //  BMS
	
	stdout = &uart0_io;
	stdin = &uart0_io;
	sei();
	
	_delay_ms(100);
	bms_request_general();

	
	while(1) {
		display_uart_puts("t0.txt=\"11.0\"\xFF\xFF\xFF");
		display_uart_puts("bar.val=70\xFF\xFF\xFF");
		
		display_set_text("t4", "wait...");
		_delay_ms(500);
		
		bms_request_general();

		_delay_ms(2500);
	}

    return 0;
}


