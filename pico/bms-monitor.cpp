#include <stdio.h>
#include <string>
#include "pico/stdlib.h"
#include "Nextion.h"
#include "SmartBMS.h"

#define NEXTION_UART_ID uart0
#define NEXTION_TX_PIN 16
#define NEXTION_RX_PIN 17

#define SMART_BMS_UART_ID uart1
#define SMART_BMS_TX_PIN 4
#define SMART_BMS_RX_PIN 5

int main() {
    Nextion nextion = Nextion(NEXTION_UART_ID, NEXTION_TX_PIN, NEXTION_RX_PIN);
    SmartBMS bms = SmartBMS(SMART_BMS_UART_ID, SMART_BMS_TX_PIN, SMART_BMS_RX_PIN);

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) {
        nextion.setText("t0", "%d.%02d", 13, 9);
        nextion.setText("t1", "test");
        nextion.setInt("bar", "val", 66);
        sleep_ms(250);
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
    }
}
