#include <stdio.h>
#include <string>
#include "pico/stdlib.h"
#include "Nextion.h"

#define NEXTION_UART_ID uart0
#define NEXTION_BAUD_RATE 115200
#define NEXTION_TX_PIN 0
#define NEXTION_RX_PIN 1

int main() {
    Nextion nextion = Nextion(NEXTION_UART_ID, NEXTION_BAUD_RATE, NEXTION_TX_PIN, NEXTION_RX_PIN);
}
