#include <stdio.h>
#include "hardware/uart.h"
#include "Nextion.h"

#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

#define END_OF_COMMAND "\xFF\xFF\xFF"
#define MAX_STRING_LENGTH 200

using namespace daniel_f_garcia::bms;

Nextion::Nextion(uart_inst_t *uartId, uint txPin, uint rxPin) {
    mUartId = uartId;
    mTxPin = txPin;
    mRxPin = rxPin;

    uart_init(mUartId, BAUD_RATE);
    gpio_set_function(mTxPin, GPIO_FUNC_UART);
    gpio_set_function(mRxPin, GPIO_FUNC_UART);
    uart_set_format(mUartId, DATA_BITS, STOP_BITS, PARITY);
}

void Nextion::setText(std::string variable, std::string text) {
    uart_puts(mUartId, variable.c_str());
    uart_puts(mUartId,".txt=\"");
    uart_puts(mUartId, text.c_str());
    uart_puts(mUartId,"\"");
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
