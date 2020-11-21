// for bitbang UART to BMS
#define DISPLAY_TX_PORT PORTD
#define DISPLAY_TX_PIN  PD7
#define DISPLAY_TX_DDR  DDRD
#define DISPLAY_TX_DDR_PIN DDD7
#define DISPLAY_MAX_STRING_LENGTH 200

void display_init(void);

void display_set_text(char* variable, const char *format, ...);

void display_set_int(char* object, char* variable, int32_t value);

void display_command(char* string);

void display_uart_put(char character);

void display_uart_puts(char* string);