#include "pico/stdlib.h"
#include "Nextion.h"
#include "SmartBMS.h"
#include "BMSDisplay.h"

#define NEXTION_UART_ID uart0
#define NEXTION_TX_PIN 16
#define NEXTION_RX_PIN 17

#define SMART_BMS_UART_ID uart1
#define SMART_BMS_TX_PIN 4
#define SMART_BMS_RX_PIN 5

using namespace daniel_f_garcia::bms;

int main() {
    Nextion nextion = Nextion(NEXTION_UART_ID, NEXTION_TX_PIN, NEXTION_RX_PIN);
    SmartBMS bms = SmartBMS(SMART_BMS_UART_ID, SMART_BMS_TX_PIN, SMART_BMS_RX_PIN);
    BMSDisplay display = BMSDisplay(nextion, bms);

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    nextion.setText("t8", "initialising...");

    sleep_ms(500);
    bms.refresh();
    sleep_ms(500);

    int i = 0;

    while (true) {
        bms.refresh();
        if (bms.isValid()) {
            display.refresh();
            nextion.setText("t8", bms.toString() + ", " + " :: " + bms.getHexResponse().substr(52));
        } else {
            nextion.setText("t8", bms.getError());
            //nextion.setText("t8", bms.getHexResponse());
        }


        sleep_ms(500);
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);

        i++;
    }
}

