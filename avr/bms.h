#define BMS_RESPONSE_TIMEOUT 100
#define BMS_READ_TIMEOUT 2
#define BUFF_SIZE   64

struct BMSGeneral {
	uint8_t  valid;
	uint16_t pack_voltage;       // multiples of 10 mV
	int16_t  pack_current;       // multiples of 10ma. Charging is positive, discharging is negative
	uint16_t nominal_capacity;   // multiples of 10mah
	uint16_t residual_capacity;  // multiples of 10mah
	uint16_t balance_status;
	int8_t   percent_capacity;   // percent, -1 if error
	int8_t   state;              // -1: discharging, 0: neither, 1: charging
	int16_t  temperature;        // celcius in multiples of 0.1 degree
	uint8_t  fet_charging;       // true if charging fet is on
	uint8_t  fet_discharging;    // true if discharging fet is on
	
	uint8_t* data;
	uint8_t  data_length;
};

struct BMSCells {
	uint8_t  valid;
	uint16_t voltage[4];       // in mV
	//uint8_t * buffer;
	//uint8_t response_length;
};

uint8_t bms_get_response_byte(uint8_t * buffer, uint8_t timeout);

uint8_t bms_get_response(uint8_t * buffer);

struct BMSGeneral bms_request_general(void);

struct BMSCells bms_request_cells(void);