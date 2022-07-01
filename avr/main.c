#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "display.h"
#include "usart.h"
#include "bms.h"

#define INVALID_THRESHOLD 2
#define CELL_OVP_1 3650
#define CELL_OVP_2 3750
#define CELL_UVP_1 2700
#define CELL_UVP_2 2500

#define PACK_OVP_1 1460
#define PACK_OVP_2 1500
#define PACK_UVP_1 1100
#define PACK_UVP_2 1000

#define PACK_CURRENT_MAX_1 1000
#define PACK_CURRENT_MAX_2 1200

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

void display_cell_voltage(char * text_field, uint16_t voltage) {
	display_set_text(text_field, "%d.%03d", voltage/1000, voltage % 1000);

	display_uart_puts(text_field);
	if (voltage>CELL_OVP_2 || voltage<CELL_UVP_2) {
		display_command(".pco=63488"); // red
	} else if (voltage>CELL_OVP_1 || voltage<CELL_UVP_1) {
		display_command(".pco=65504"); // yellow
	} else {
		display_command(".pco=65535"); // white
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
	_delay_ms(100);
	
	uint8_t general_invalid_count = 0;
	uint8_t cell_invalid_count = 0;
	
	while(1) {
		struct BMSGeneral bms_general = bms_request_general();
		
		if (bms_general.valid) {
			general_invalid_count = 0;
			
			// Pack Voltage
			display_set_text("t0", "%d.%02d", bms_general.pack_voltage/100, bms_general.pack_voltage % 100);
			if (bms_general.pack_voltage>PACK_OVP_2 || bms_general.pack_voltage<PACK_UVP_2) {
				display_command("t0.pco=63488"); // red
			} else if (bms_general.pack_voltage>PACK_OVP_1 || bms_general.pack_voltage<PACK_UVP_1) {
				display_command("t0.pco=65504"); // yellow
			} else {
				display_command("t0.pco=65535"); // white
			}
			
			// Current
			if (bms_general.state==-1) {
				display_set_text("t1", "-%d.%02d", bms_general.pack_current/100, bms_general.pack_current % 100);
			} else {
				display_set_text("t1", "%d.%02d", bms_general.pack_current/100, bms_general.pack_current % 100);
			}
			if (bms_general.pack_current>PACK_CURRENT_MAX_2) {
				display_command("t1.pco=63488"); // red
			} else if (bms_general.pack_current>PACK_CURRENT_MAX_1) {
				display_command("t1.pco=65504"); // yellow
			} else {
				display_command("t1.pco=65535"); // white
			}
			
			// Percent Capacity
			if (bms_general.percent_capacity==-1) {
				display_set_text("t2", "ERR");
			} else {
				display_set_text("t2", "%01d", bms_general.percent_capacity);
				display_set_int("bar", "val", bms_general.percent_capacity);
			}
			
			// Temperature
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
			
			// Balance Status
			if (bms_general.balance_status & _BV(0)) {
				display_command("vis p0,1");
			} else {
				display_command("vis p0,0");
			}
			
			if (bms_general.balance_status & _BV(1)) {
				display_command("vis p1,1");
			} else {
				display_command("vis p1,0");
			}
			
			if (bms_general.balance_status & _BV(2)) {
				display_command("vis p2,1");
			} else {
				display_command("vis p2,0");
			}
			
			if (bms_general.balance_status & _BV(3)) {
				display_command("vis p3,1");
			} else {
				display_command("vis p3,0");
			}
			
			//display_set_text("t8", tohex(bms_general.data, bms_general.data_length));
			
			// Charge/Discharge FETs
			if (bms_general.fet_charging) {
				display_command("vis p4,1");
			} else {
				display_command("vis p4,0");
			}
			if (bms_general.fet_discharging) {
				display_command("vis p5,1");
			} else {
				display_command("vis p5,0");
			}
			
		} else {
			if (general_invalid_count<INVALID_THRESHOLD) {
				general_invalid_count++;
			} else {
				display_set_text("t0", ". . . .");
				display_set_text("t1", ". . . .");	
				display_set_text("t2", ". . . .");	
				display_set_text("t3", ". . . .");	
			}
			
		}
		
		_delay_ms(50);
		
		struct BMSCells bms_cells = bms_request_cells();
		
		//char * temp = tohex(bms_cells.buffer, bms_cells.response_length);
		//display_set_text("t8", "%d: %s", bms_cells.response_length, temp);
		
		if (bms_cells.valid) {
			cell_invalid_count = 0;
			display_cell_voltage("t4", bms_cells.voltage[0]);
			display_cell_voltage("t5", bms_cells.voltage[1]);
			display_cell_voltage("t6", bms_cells.voltage[2]);
			display_cell_voltage("t7", bms_cells.voltage[3]);
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


