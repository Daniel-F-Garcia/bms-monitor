// for bitbang UART to BMS
#define DISPLAY_TX_PORT PORTB
#define DISPLAY_TX_PIN  PB1
#define DISPLAY_TX_DDR  DDRB
#define DISPLAY_TX_DDR_PIN DDB1
#define DISPLAY_MAX_STRING_LENGTH 200

void display_init(void);

void display_set_text(char* variable, const char *format, ...);

void display_uart_put(char character);

void display_uart_puts(char* string);