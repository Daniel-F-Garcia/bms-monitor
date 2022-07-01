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

struct BMSGeneral bms_request_general() {
	struct BMSGeneral ret;
	ret.valid = 0;
	
	unsigned char request[] = {0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77};
	for (int i=0; i<7; i++) {
		uart_putc(request[i]);
	}

	uint8_t buffer[BUFF_SIZE];
	uint8_t response_length = bms_get_response(buffer);

	if (response_length<4) {
		return ret;
	}
	
	if (buffer[0]!=0xDD || buffer[1]!=0x03 || buffer[2]!=0x00 || buffer[3]<0x19) {
		return ret;
	}
	
	// Data addresses https://gitlab.com/bms-tools/bms-tools/-/blob/master/JBD_REGISTER_MAP.md
	// Exceptions: Fet Status @ 0x14, NTC @ 0x17 
	
	uint8_t* data = buffer + 4;
	ret.data = data;
	ret.data_length = response_length - 4;
		
	ret.pack_voltage = (data[0x00] << 8) | data[0x01];
	ret.pack_current = (data[0x02] << 8) | data[0x03];
	if (ret.pack_current==0) {
		ret.state = 0;
	} else if (ret.pack_current>0) {
		ret.state = 1;
	} else {
		ret.state = -1;
	}
	ret.pack_current = abs(ret.pack_current);
	ret.residual_capacity = (data[0x04] << 8) | data[0x05];
	ret.nominal_capacity = (data[0x06] << 8) | data[0x07];
	if (ret.nominal_capacity==0) {
		ret.percent_capacity = -1;
	} else {
		ret.percent_capacity = (uint32_t)ret.residual_capacity * 100 / ret.nominal_capacity;
	}
	ret.balance_status = (data[0x0C] << 8) | data[0x0D];
	ret.temperature = ((data[0x17] << 8) | data[0x18]) - 2731;
	ret.fet_charging = data[0x14] & 0x01;
	ret.fet_discharging = (data[0x14] & 0x02) >> 1;
	ret.valid = 1;
	
	return ret;
}

struct BMSCells bms_request_cells() {
	struct BMSCells ret;
	ret.valid = 0;
	
	//uart_flush();
	unsigned char data[] = {0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77};
	for (int i=0; i<7; i++) {
		uart_putc(data[i]);
	}

	uint8_t buffer[BUFF_SIZE];
	uint8_t response_length = bms_get_response(buffer);
	
	//ret.buffer = buffer;
	//ret.response_length = response_length;

	if (response_length<4) {
		return ret;
	}
	
	if (buffer[0]!=0xDD || buffer[1]!=0x04 || buffer[2]!=0x00 || buffer[3]<0x08) {
		return ret;
	}
	
	ret.voltage[0] = (buffer[4] << 8) | buffer[5];
	ret.voltage[1] = (buffer[6] << 8) | buffer[7];
	ret.voltage[2] = (buffer[8] << 8) | buffer[9];
	ret.voltage[3] = (buffer[10] << 8) | buffer[11];
	
	ret.valid = 1;
	
	return ret;
}

