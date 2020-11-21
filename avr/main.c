#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "display.h"
#include "usart.h"
#include "bms.h"

#define INVALID_THRESHOLD 2
#define CELL_OVP 3750
#define CELL_UVP 2700
// Celcius multiples of 0.1
#define TEMP_WARN 450
#define TEMP_OVER 600


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

int main(void) {
	display_init();
	uart_set_FrameFormat(USART_8BIT_DATA|USART_1STOP_BIT|USART_NO_PARITY|USART_ASYNC_MODE);
	uart_init(BAUD_CALC(9600));
	
	stdout = &uart0_io;
	stdin = &uart0_io;
	sei();
	
	_delay_ms(100);
	bms_request_general();
	_delay_ms(100);
	
	uint8_t general_invalid_count = 0;
	uint8_t cell_invalid_count = 0;
	
	while(1) {
		struct BMSGeneral bms_general = bms_request_general();
		
		if (bms_general.valid) {
			general_invalid_count = 0;
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
				display_set_int("bar", "val", bms_general.percent_capacity);
			}
			if (bms_general.temperature<0) {
				display_set_text("t3", "-%d.%01d", bms_general.temperature/10, bms_general.temperature % 10);
			} else {
				display_set_text("t3", "%d.%01d", bms_general.temperature/10, bms_general.temperature % 10);
			}
			if (bms_general.temperature>TEMP_OVER) {
				display_command("t3.pco=63488"); // red
			} else if (bms_general.temperature>TEMP_WARN) {
				display_command("t3.pco=65504"); // yellow
			} else {
				display_command("t3.pco=65535"); // white
			}
			
		} else {
			if (general_invalid_count<INVALID_THRESHOLD) {
				general_invalid_count++;
			} else {
				display_set_text("t0", ". . . .");
				display_set_text("t1", ". . . .");	
				display_set_text("t2", ". . .");	
				display_set_text("t3", ". . .");	
			}
			
		}
		
		_delay_ms(50);
		
		struct BMSCells bms_cells = bms_request_cells();
		
		//char * temp = tohex(bms_cells.buffer, bms_cells.response_length);
		//display_set_text("t8", "%d: %s", bms_cells.response_length, temp);
		
		if (bms_cells.valid) {
			cell_invalid_count = 0;
			display_set_text("t4", "%d.%03d", bms_cells.voltage[0]/1000, bms_cells.voltage[0] % 1000);
			display_set_text("t5", "%d.%03d", bms_cells.voltage[1]/1000, bms_cells.voltage[1] % 1000);
			display_set_text("t6", "%d.%03d", bms_cells.voltage[2]/1000, bms_cells.voltage[2] % 1000);
			display_set_text("t7", "%d.%03d", bms_cells.voltage[3]/1000, bms_cells.voltage[3] % 1000);
			
			if (bms_cells.voltage[0]>CELL_OVP || bms_cells.voltage[0]<CELL_UVP) {
				display_command("t4.pco=65504"); // yellow
			} else {
				display_command("t4.pco=65535"); // white
			}
			
			if (bms_cells.voltage[1]>CELL_OVP || bms_cells.voltage[1]<CELL_UVP) {
				display_command("t5.pco=65504"); // yellow
			} else {
				display_command("t5.pco=65535"); // white
			}
			
			if (bms_cells.voltage[2]>CELL_OVP || bms_cells.voltage[2]<CELL_UVP) {
				display_command("t6.pco=65504"); // yellow
			} else {
				display_command("t6.pco=65535"); // white
			}
			
			if (bms_cells.voltage[3]>CELL_OVP || bms_cells.voltage[3]<CELL_UVP) {
				display_command("t7.pco=65504"); // yellow
			} else {
				display_command("t7.pco=65535"); // white
			}
		} else {
			if (cell_invalid_count<INVALID_THRESHOLD) {
				cell_invalid_count++;
			} else {
				display_set_text("t4", ". . . . .");
				display_set_text("t5", ". . . . .");
				display_set_text("t6", ". . . . .");
				display_set_text("t7", ". . . . .");
			}
		}
		
		_delay_ms(2500);
	}

    return 0;
}


