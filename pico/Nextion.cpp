#include <stdio.h>
#include <cstdarg>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "Nextion.h"

#define END_OF_COMMAND "\xFF\xFF\xFF"
#define MAX_STRING_LENGTH 200

Nextion::Nextion(uart_inst_t *uartId, uint baudRate, uint txPin, uint rxPin) {
    mUartId = uartId;
    mBaudRate = baudRate;
    mTxPin = txPin;
    mRxPin = rxPin;

    uart_init(mUartId, mBaudRate);
    gpio_set_function(mTxPin, GPIO_FUNC_UART);
    gpio_set_function(mRxPin, GPIO_FUNC_UART);
}

void Nextion::setText(std::string variable, std::string format, ...) {
    char value[MAX_STRING_LENGTH];

    va_list args;
    va_start(args, format);
    vsprintf(value, format.c_str(), args);
    va_end(args);

    uart_puts(mUartId, variable.c_str());
    uart_puts(mUartId,".txt=\"");
    uart_puts(mUartId, value);
    uart_puts(mUartId, END_OF_COMMAND);
}

void Nextion::setInt(std::string object, std::string variable, int value) {
    uart_puts(mUartId, object.c_str());
    uart_puts(mUartId, ".");
    uart_puts(mUartId, variable.c_str());
    uart_puts(mUartId, "=");
    uart_puts(mUartId, std::to_string(value).c_str());
    uart_puts(mUartId, END_OF_COMMAND);
}

void Nextion::command(std::string command) {
    uart_puts(mUartId, command.c_str());
    uart_puts(mUartId, END_OF_COMMAND);
}





//void display_set_text(char* variable, const char *format, ...) {
//}

