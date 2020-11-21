#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "usart.h"
#include "bms.h"

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
	
	while (status==COMPLETED) {
		current++;
		response_length++;
		status = bms_get_response_byte(current, BMS_READ_TIMEOUT);
	}
	
	return response_length;
}